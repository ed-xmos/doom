#include <stdio.h>
#include <platform.h>
#include <string.h>
#include <xs1.h>
#include "doom_audio.h"

in port p_buttons = on tile[0]: XS1_PORT_4D;

extern unsafe streaming chanend g_c_midi_app;

int doom_audio_register_song(const uint8_t * data, size_t len){
  unsafe{
	  g_c_midi_app <: DA_REGISTER_SONG;
	  g_c_midi_app <: len;
	  for(int i=0; i < len; i++){
	  	(streaming chanend)g_c_midi_app <: data[i];
	  }
	  int handle;
	  g_c_midi_app :> handle;
	  printf("doom_audio_register_song handle: %d len: %d\n", handle, len);

	  return handle;
  }
}

void doom_audio_play_song(int handle, int looping){
  printf("doom_audio_play_song handle: %d looping: %d\n", handle, looping);
	unsafe{
	  g_c_midi_app <: DA_PLAY_SONG;
	  g_c_midi_app <: handle;
	  g_c_midi_app <: looping;
	}	  
}


void doom_audio_pause_song(int handle){
  printf("doom_audio_play_song handle: %d\n", handle);
	unsafe{
	  g_c_midi_app <: DA_PAUSE_SONG;
	  g_c_midi_app <: handle;
	}
}

void doom_audio_resume_song(int handle){
  printf("doom_audio_resume_song handle: %d\n", handle);
	unsafe{
	  g_c_midi_app <: DA_RESUME_SONG;
	  g_c_midi_app <: handle;
	}
}

void doom_audio_stop_song(int handle){
  printf("doom_audio_stop_song handle: %d\n", handle);
	unsafe{
	  g_c_midi_app <: DA_STOP_SONG;
	  g_c_midi_app <: handle;
	}
}

void doom_audio_unregister_song(int handle){
  printf("doom_audio_unregister_song handle: %d\n", handle);
	unsafe{
	  g_c_midi_app <: DA_UNREGISTER_SONG;
	  g_c_midi_app <: handle;
	}
}

extern void I_UpdateSound(void * unsafe unused, uint8_t * unsafe stream, int len);
extern int pcm_initialised;


void pcm_samples_server(streaming chanend c_pcm_app){
	int16_t stream[SAMPLECOUNT][APP_NUM_I2S_CHANNELS_OUT] = {{0}};
	int master_volume = 15;

	int port_old;
	p_buttons :> port_old;

	while(1){
		select{
			case c_pcm_app :> int _:
				unsafe{
					// printf("Samples requested\n");
					c_pcm_app <: master_volume;

					if(!pcm_initialised){
						printf("PCM not initialised yet..\n");
					} else {
						I_UpdateSound(NULL, (uint8_t * unsafe)stream, SAMPLECOUNT * APP_NUM_I2S_CHANNELS_OUT * sizeof(int16_t));
					}
					
					int16_t * unsafe ptr = (int16_t * unsafe)stream[0];
					for(int i = 0; i < SAMPLECOUNT * APP_NUM_I2S_CHANNELS_OUT; i++){
				    c_pcm_app <: (int32_t)*ptr;
				    ptr++;
					}
				}
			break;
			
			case p_buttons when pinsneq(port_old) :> int new_port:
				port_old = new_port;
				if(new_port == 0x02){
					if(master_volume < 15) master_volume++;
					printf("Master volume: %d\n", master_volume);
				}
				else if(new_port == 0x01){
					if(master_volume > 0) master_volume--;
					printf("Master volume: %d\n", master_volume);
				}
			break;
		}
	}
}