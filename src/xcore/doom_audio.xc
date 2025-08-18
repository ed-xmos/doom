#include <stdio.h>
#include <string.h>
#include <xs1.h>
#include "doom_audio.h"


extern unsafe streaming chanend g_c_midi_app;
extern unsafe streaming chanend g_c_pcm_app;

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


void doom_audio_send_pcm_sample_buffer(void){
	unsafe{
		select{
			case g_c_pcm_app :> int _:
				printf("Samples requested\n");
				int16_t stream[SAMPLECOUNT][APP_NUM_I2S_CHANNELS_OUT];
				I_UpdateSound(NULL, (uint8_t * unsafe)stream, SAMPLECOUNT);
				for(int i = 0; i < 16; i++){
					int16_t *ptr = stream[i];
					printf("sample %d: %d\n", i, *ptr);
				}
				break;
				
			default:
				break;
		}
	}
}