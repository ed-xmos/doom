#ifndef _doom_audio_h_
#define _doom_audio_h_

#include <stdint.h>
#include "app_audio_config.h"

#ifdef __cplusplus
extern "C" {
#endif
int doom_audio_register_song(const uint8_t *data, size_t len);
void doom_audio_play_song(int handle, int looping);
void doom_audio_pause_song(int handle);
void doom_audio_resume_song(int handle);
void doom_audio_stop_song(int handle);
void doom_audio_unregister_song(int handle);
void doom_audio_send_pcm_sample_buffer(void);
#ifdef __cplusplus
}
#endif

// commands to audio
#define DA_REGISTER_SONG 	0
#define DA_PLAY_SONG 		1	
#define DA_PAUSE_SONG 		2
#define DA_RESUME_SONG 		3	
#define DA_STOP_SONG 		4	
#define DA_UNREGISTER_SONG  5	


#endif