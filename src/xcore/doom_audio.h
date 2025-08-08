#ifndef _doom_audio_h_
#define _doom_audio_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
void doom_audio_register_song(const uint8_t *data, size_t len);
void doom_audio_play_song(int handle, int looping);
#ifdef __cplusplus
}
#endif

// commands to audio
#define DA_REGISTER_SONG 	0
#define DA_PLAY_SONG 		1	


#endif