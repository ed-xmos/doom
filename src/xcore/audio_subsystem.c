// Copyright 2022-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <xscope.h>
#include <xs1.h>
#include <xcore/assert.h>
#include <xcore/channel.h>
#include <xcore/channel_streaming.h>
#include <xcore/hwtimer.h>
#include <xcore/select.h>
#include <platform.h>
#include <math.h>

#include "app_audio_config.h"
#include "i2s.h"
#include "compiler.h"
#include "spi.h"
#include "xk_evk_xu316/board.h"
#define double float
#define TML_IMPLEMENTATION
#include "tml.h"
#include "midi_msg.h"
#include "doom_audio.h"


// Holds global MIDI playback state
static double g_Msec;               //current playback time
static tml_message* g_MidiMessage;  //next message to be played


size_t ProduceMIDIToBuffer(const uint8_t *musBuf, size_t musSize, uint8_t *outBuf, size_t outBufSize);
void render_midi(chanend c_midi_pcm, chanend c_midi_msg);

DECLARE_JOB(render_midi_wrapper, (chanend_t, chanend_t));
void render_midi_wrapper(chanend c_midi_pcm, chanend c_midi_msg){
    render_midi(c_midi_pcm, c_midi_msg);
}



#define N_SINE 50
typedef struct i2s_callback_args_t {
    bool did_restart;                       // Set by init
    int32_t samples[APP_NUM_I2S_CHANNELS_OUT][N_SINE];
    int table_idx;
    chanend_t c_midi_pcm;
} i2s_callback_args_t;



I2S_CALLBACK_ATTR
static void i2s_init(void *app_data, i2s_config_t *i2s_config){
    printf("I2S init\n");
    i2s_callback_args_t *cb_args = app_data;

    i2s_config->mode = I2S_MODE_I2S;
    i2s_config->mclk_bclk_ratio = (APP_MCLK_FREQUENCY / (APP_I2S_FREQUENCY * 64));

    cb_args->did_restart = true;
}

I2S_CALLBACK_ATTR
static i2s_restart_t i2s_restart_check(void *app_data){
    i2s_callback_args_t *cb_args = app_data;
    (void)cb_args;

    return I2S_NO_RESTART;
}


I2S_CALLBACK_ATTR
static void i2s_send(void *app_data, size_t num_out, int32_t *i2s_sample_buf){
    i2s_callback_args_t *cb_args = app_data;

    static int counter = 0;

    // Non-blocking read to wait for pointer to samples
    SELECT_RES(
        CASE_THEN(cb_args->c_midi_pcm, midi_samples_available),
        DEFAULT_THEN(default_handler)
    )
    {
        midi_samples_available:
        {
            i2s_sample_buf[0] = chan_in_word(cb_args->c_midi_pcm);
            i2s_sample_buf[1] = chan_in_word(cb_args->c_midi_pcm);
        }
        break;

        default_handler:
        {
            i2s_sample_buf[1] = 0;
            if(++counter == APP_MIDI_SAMPLE_RATE){
                puts("S");
                counter = 0;
            }
        }
        break;
    }
}

I2S_CALLBACK_ATTR
static void i2s_receive(void *app_data, size_t num_in, const int32_t *i2s_sample_buf){
    i2s_callback_args_t *cb_args = app_data;
    (void)cb_args;
}


DECLARE_JOB(i2s_task, (chanend_t, chanend_t));
void i2s_task(chanend_t c_midi_pcm, chanend_t c_i2c){
    
    // Setup DAC
    // Board configuration from lib_board_support
    static const xk_evk_xu316_config_t hw_config = {
            APP_MCLK_FREQUENCY * 2// default_mclk
    };

    hwtimer_realloc_xc_timer();
    xk_evk_xu316_AudioHwChanInit(c_i2c);
    xk_evk_xu316_AudioHwInit(&hw_config);
    xk_evk_xu316_AudioHwConfig(APP_I2S_FREQUENCY, hw_config.default_mclk, 0, 24, 24);


    // I2S resources
    port_t p_i2s_dout[APP_NUM_I2S_LINES_OUT] = {PORT_I2S_DAC_DATA};
    port_t p_bclk = PORT_I2S_BCLK;
    port_t p_lrclk = PORT_I2S_LRCLK;
    port_t p_mclk = PORT_MCLK_IN;

    xclock_t i2s_ck_bclk = XS1_CLKBLK_5;

    port_enable(p_mclk);
    port_enable(p_bclk);
    // NOTE:  p_lrclk does not need to be enabled by the caller

    // Initialise app_data
    i2s_callback_args_t app_data = {
        .did_restart = false,
        .samples = {{0}},
        .table_idx = 0,
        .c_midi_pcm = c_midi_pcm
    };

    for(int i = 0; i < N_SINE; i++){
        int32_t sample = ((1 << 24) * sin(6.283185307 / N_SINE * i));
        app_data.samples[0][i]= sample;
        app_data.samples[1][i]= sample;
    }

    // Initialise callback function pointers
    i2s_callback_group_t i2s_cb_group = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = &app_data,
    };

    printf("Starting I2S master\n");

    i2s_master(
            &i2s_cb_group,
            p_i2s_dout,
            APP_NUM_I2S_LINES_OUT,
            NULL,
            0,
            p_bclk,
            p_lrclk,
            p_mclk,
            i2s_ck_bclk);
}



size_t midi_size[NUM_MIDI_TRACKS] = {0};
uint8_t midi_data[NUM_MIDI_TRACKS][MAX_MIDI_TRACK_SIZE] = {{0}};
int midi_register_handle = 0;

DECLARE_JOB(sound_dispatcher, (chanend_t, chanend_t));
void sound_dispatcher(chanend_t c_audio, chanend_t c_midi_track){
    uint8_t mus_data[MAX_MIDI_TRACK_SIZE] = {0};

    while(1){
        int cmd = s_chan_in_word(c_audio);

        switch(cmd){
            case DA_REGISTER_SONG:{
                size_t mus_len = s_chan_in_word(c_audio);
                printf("DA_REGISTER_SONG handle: %d len: %d\n", midi_register_handle, mus_len);
                s_chan_in_buf_byte(c_audio, mus_data, mus_len);

                for(int i = 0; i < 16; i++)printf("0x%x, ", mus_data[i]);
                printf("\n");

                midi_size[midi_register_handle] = ProduceMIDIToBuffer(mus_data, mus_len, midi_data[midi_register_handle], MAX_MIDI_TRACK_SIZE);

                // ACK to say done and return handle
                s_chan_out_word(c_audio, midi_register_handle);

                if(++midi_register_handle == NUM_MIDI_TRACKS){
                    midi_register_handle = 0;
                }

                if(midi_size > 0) {
                    printf("MIDI converted!\n");
                }

                break;
            }
            case DA_PLAY_SONG:{
                int handle = s_chan_in_word(c_audio);
                int looping = s_chan_in_word(c_audio);
                printf("DA_PLAY_SONG: %d %d\n", handle, looping);
                chan_out_word(c_midi_track, handle);
                chan_out_word(c_midi_track, looping);

                break;
            }

            case DA_PAUSE_SONG:{
                int handle = s_chan_in_word(c_audio);
                printf("DA_PAUSE_SONG: %d\n", handle);
  
                break;
            }
  
            case DA_RESUME_SONG:{
                int handle = s_chan_in_word(c_audio);
                printf("DA_RESUME_SONG: %d\n", handle);
  
                break;
            }
  
            case DA_STOP_SONG:{
                int handle = s_chan_in_word(c_audio);
                printf("DA_STOP_SONG: %d\n", handle);
  
                break;
            }
  
            case DA_UNREGISTER_SONG:{
                int handle = s_chan_in_word(c_audio);
                printf("DA_UNREGISTER_SONG: %d\n", handle);
  
                break;
            }
        }
    }
}

DECLARE_JOB(midi_sequencer, (chanend_t, chanend_t));
void midi_sequencer(chanend_t c_midi_msg, chanend_t c_midi_track){
    int print_midi_events = 0;
    hwtimer_t tmr = hwtimer_alloc();
    int time_trigger = 0;

    tml_message* TinyMidiLoader = NULL;

    // get first msg
    int midi_handle = chan_in_word(c_midi_track);
    int looping = chan_in_word(c_midi_track);

    // Forever MIDI sequencer loop
    while(1){
        printf("MIDI loading handle %d size: %d\n", midi_handle, midi_size[midi_handle]);

#if WRITE_MIDI_FILE
        FILE *write_ptr;
        write_ptr = fopen("test.mid","wb");  // w for write, b for binary
        fwrite(midi_data[midi_handle], midi_size[midi_handle], 1, write_ptr); //
        printf("FILE WRITTEN\n");
#endif

        TinyMidiLoader = tml_load_memory(midi_data[midi_handle], midi_size[midi_handle]);
        if (!TinyMidiLoader)
        {
            fprintf(stderr, "Could not load MIDI file\n");
            g_MidiMessage->next = NULL;
        } else {
            printf("MIDI loaded\n");
            g_MidiMessage = TinyMidiLoader;
        }

        //Set up the global MidiMessage pointer to the first MIDI message
        g_Msec = 0.0;
        time_trigger = hwtimer_get_time(tmr);

        //Wait until the entire MIDI file has been played back (until the end of the linked message list is reached)
        g_MidiMessage = g_MidiMessage->next;

        while (g_MidiMessage != NULL){

            //Loop through al[l MIDI messages which need to be played up until the current playback time
            if(g_Msec >= g_MidiMessage->time)
            {
                switch (g_MidiMessage->type)
                {
                    case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
                        if(print_midi_events) printf("TML_PROGRAM_CHANGE %d %d\n", g_MidiMessage->channel, g_MidiMessage->program);
                        chan_out_word(c_midi_msg, PROGRAM_CHANGE);
                        chan_out_word(c_midi_msg, g_MidiMessage->channel);
                        chan_out_word(c_midi_msg, g_MidiMessage->program);
                        break;

                    case TML_NOTE_ON: //play a note
                        if(print_midi_events) printf("TML_NOTE_ON %d %d %d\n", g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity);
                        chan_out_word(c_midi_msg, NOTE_ON);
                        chan_out_word(c_midi_msg, g_MidiMessage->channel);
                        chan_out_word(c_midi_msg, g_MidiMessage->key);
                        chan_out_word(c_midi_msg, g_MidiMessage->velocity);
                        break;

                    case TML_NOTE_OFF: //stop a note
                        if(print_midi_events) printf("TML_NOTE_OFF %d %d\n", g_MidiMessage->channel, g_MidiMessage->key);
                        chan_out_word(c_midi_msg, NOTE_OFF);
                        chan_out_word(c_midi_msg, g_MidiMessage->channel);
                        chan_out_word(c_midi_msg, g_MidiMessage->key);
                        break;

                    case TML_PITCH_BEND: //pitch wheel modification
                        if(print_midi_events) printf("TML_PITCH_BEND %d %d\n", g_MidiMessage->channel, g_MidiMessage->pitch_bend);
                        chan_out_word(c_midi_msg, NOTE_OFF);
                        chan_out_word(c_midi_msg, g_MidiMessage->channel);
                        chan_out_word(c_midi_msg, g_MidiMessage->pitch_bend);
                        break;

                    case TML_CONTROL_CHANGE: //MIDI controller messages
                        if(print_midi_events) printf("TML_CONTROL_CHANGE %d %d %d\n", g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
                        chan_out_word(c_midi_msg, CONTROL_CHANGE);
                        chan_out_word(c_midi_msg, g_MidiMessage->channel);
                        chan_out_word(c_midi_msg, g_MidiMessage->control);
                        chan_out_word(c_midi_msg, g_MidiMessage->control_value);
                        break;
                }

                g_MidiMessage = g_MidiMessage->next;

            }

            g_Msec += 1000.0 / APP_MIDI_SAMPLE_RATE;
            time_trigger += XS1_TIMER_HZ / APP_MIDI_SAMPLE_RATE;
            hwtimer_set_trigger_time(tmr, time_trigger);

            SELECT_RES(
                CASE_THEN(tmr, next_midi_event),
                CASE_THEN(c_midi_track, new_midi_track)
            )
            {
                next_midi_event:
                {
                    // Do nothing, just drop through and keep going in midi loop
                }
                break;

                new_midi_track:
                {
                    midi_handle = chan_in_word(c_midi_track);
                    looping = chan_in_word(c_midi_track);
                    const int numVoices = 16; // TODO grab from c++
                    for(int channel = 0; channel < numVoices; channel++){
                        // TODO WHY DOESNT THIS WORK?
                        // chan_out_word(c_midi_msg, CONTROL_CHANGE);
                        // chan_out_word(c_midi_msg, 0xb0 + channel);
                        // chan_out_word(c_midi_msg, 0x7B); // All notes off
                        // chan_out_word(c_midi_msg, 0);
                        for(int note = 0; note < 128; note++){
                            chan_out_word(c_midi_msg, NOTE_OFF);
                            chan_out_word(c_midi_msg, channel);
                            chan_out_word(c_midi_msg, note);
                        }
                    }

                    g_MidiMessage = NULL; // Break loop and reset stuff
                }
                break;
            } // select
        } // while (g_MidiMessage != NULL);
    } // while (1);
}

void audio_subsystem(chanend_t c_i2c,
                     chanend_t c_audio)
{
    channel_t c_midi_pcm = chan_alloc();
    channel_t c_midi_msg = chan_alloc();
    channel_t c_midi_track = chan_alloc();


    PAR_JOBS(PJOB(render_midi_wrapper, (c_midi_pcm.end_a, c_midi_msg.end_b)),
             PJOB(midi_sequencer, (c_midi_msg.end_a, c_midi_track.end_a)),
             PJOB(i2s_task, (c_midi_pcm.end_b, c_i2c)),
             PJOB(sound_dispatcher, (c_audio, c_midi_track.end_b)));

}
