#include <stdio.h>
#include <xccompat.h>
#include <xcore/select.h>
#include "midi.h"
#include "midisynth.h"
#include "instruments.h"
#include "instruments_generated.h"
#include "midi_msg.h"

// prevent mangling so we can call from XC
extern "C" {
    void render_midi(chanend c_midi_pcm, chanend c_midi_msg);
}

MidiSynth synth;
chanend Dac::c_midi;
int32_t Dac::left;
int32_t Dac::right;

// The below thunks are invoked during Midi::Dispatch() and forwarded to our MidiSynth.
void noteOn(uint8_t channel, uint8_t note, uint8_t velocity)		    { synth.midiNoteOn(channel, note, velocity); }
void noteOff(uint8_t channel, uint8_t note)							            { synth.midiNoteOff(channel, note); }
void sysex(uint8_t cbData, uint8_t data[])							            { /* do nothing */ }
void controlChange(uint8_t channel, uint8_t control, uint8_t value) { synth.midiControlChange(channel, control, value); }
void programChange(uint8_t channel, uint8_t value)					        { synth.midiProgramChange(channel, value); }
void pitchBend(uint8_t channel, int16_t value)						          { synth.midiPitchBend(channel, value); }

void receive_midi_messgage(chanend c_midi_msg){
	int32_t cmd;
	
	SELECT_RES(
	    CASE_THEN(c_midi_msg, midi_msg_available),
	    DEFAULT_THEN(default_handler)
	)
	{
	    midi_msg_available:
	    {
	        cmd = chan_in_word(c_midi_msg);
	        switch(cmd){
	        	case NOTE_ON:{
	        		int32_t channel = chan_in_word(c_midi_msg);
	        		int32_t note = chan_in_word(c_midi_msg);
	        		int32_t velocity = chan_in_word(c_midi_msg);
	        		noteOn(channel, note, velocity);
	        		break;
	        	}
	        	case NOTE_OFF:{
	        		int32_t channel = chan_in_word(c_midi_msg);
	        		int32_t note = chan_in_word(c_midi_msg);
	        		noteOff(channel, note);
	        		break;
	        	}
	        	case SYSEX:
	        		break;
	        	case CONTROL_CHANGE:{
	        		int32_t channel = chan_in_word(c_midi_msg);
	        		int32_t control = chan_in_word(c_midi_msg);
	        		int32_t value = chan_in_word(c_midi_msg);
	        		controlChange(channel, control, value);
	        		break;
	        	}
	        	case PROGRAM_CHANGE:{
	        		int32_t channel = chan_in_word(c_midi_msg);
	        		int32_t value = chan_in_word(c_midi_msg);
	        		programChange(channel, value);
	        		break;
	        	}
	        	case PITCH_BEND:{
	        		int32_t channel = chan_in_word(c_midi_msg);
	        		int32_t value = chan_in_word(c_midi_msg);
	        		pitchBend(channel, value);
	        		break;
	        	}
	    	}
	    break;

	    default_handler:
	    {
	    	// Drop through if no message
	    }
	    break;
	
		}
	}
}

void render_midi(chanend c_midi_pcm, chanend c_midi_msg){

	printf("run_midi!\n");

	Dac::setup(c_midi_pcm);

    synth.begin();

    // The lib is not happy unless you init the voices
	for (int i = 0; i < Synth::numVoices; i++) {
	    uint8_t note = 60 + 4 * i; // Use different notes for clarity
		noteOn(i, note, 0);
	}

	while(1){
		receive_midi_messgage(c_midi_msg);
		synth.isr();
	}

}