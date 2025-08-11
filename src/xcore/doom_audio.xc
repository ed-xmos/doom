#include <stdio.h>
#include <string.h>
#include <xs1.h>

#include "doom_audio.h"


extern unsafe streaming chanend g_c_audio;

int doom_audio_register_song(const uint8_t * data, size_t len){
  unsafe{
	  g_c_audio <: DA_REGISTER_SONG;
	  g_c_audio <: len;
	  for(int i=0; i < len; i++){
	  	(streaming chanend)g_c_audio <: data[i];
	  }
	  int handle;
	  g_c_audio :> handle;
	  printf("doom_audio_register_song handle: %d len: %d\n", handle, len);

	  return handle;
  }
}

void doom_audio_play_song(int handle, int looping){
  printf("doom_audio_play_song handle: %d looping: %d\n", handle, looping);
	unsafe{
	  g_c_audio <: DA_PLAY_SONG;
	  g_c_audio <: handle;
	  g_c_audio <: looping;
	}	  
}


void doom_audio_pause_song(int handle){
  printf("doom_audio_play_song handle: %d\n", handle);
	unsafe{
	  g_c_audio <: DA_PAUSE_SONG;
	  g_c_audio <: handle;
	}
}

void doom_audio_resume_song(int handle){
  printf("doom_audio_resume_song handle: %d\n", handle);
	unsafe{
	  g_c_audio <: DA_RESUME_SONG;
	  g_c_audio <: handle;
	}
}

void doom_audio_stop_song(int handle){
  printf("doom_audio_stop_song handle: %d\n", handle);
	unsafe{
	  g_c_audio <: DA_STOP_SONG;
	  g_c_audio <: handle;
	}
}

void doom_audio_unregister_song(int handle){
  printf("doom_audio_unregister_song handle: %d\n", handle);
	unsafe{
	  g_c_audio <: DA_UNREGISTER_SONG;
	  g_c_audio <: handle;
	}
}
