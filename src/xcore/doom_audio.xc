#include <stdio.h>
#include <string.h>
#include <xs1.h>

#include "doom_audio.h"


extern unsafe streaming chanend g_c_audio;

void doom_audio_register_song(const uint8_t * data, size_t len){
  printf("I_RegisterSong  len: %d\n", len);
  unsafe{
	  g_c_audio <: DA_REGISTER_SONG;
	  g_c_audio <: len;
	  sout_char_array((streaming chanend)g_c_audio, data, len);
  }
}
void doom_audio_play_song(int handle, int looping){
  printf("I_PlaySong handle: %d looping: %d\n", handle, looping);
	unsafe{
	  g_c_audio <: DA_PLAY_SONG;
	  g_c_audio <: handle;
	  g_c_audio <: looping;
	}	  
}
