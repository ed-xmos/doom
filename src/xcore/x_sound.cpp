// Just enough to avoid undefined references.

#include "sounds.h"
#include "compiler.h"
#include "doomtype.h"
#include "w_wad.h"
#include <stddef.h>
#include <stdio.h>
#include "doom_audio.h"

#define NUM_CHANNELS		8


static void *music[2] = { NULL, NULL };

int snd_card = 1;
int mus_card = 1;
int detect_voices = 0;
int 		lengths[NUMSFX];
unsigned int	channelstep[NUM_CHANNELS];
unsigned int	channelstepremainder[NUM_CHANNELS];
unsigned char*	channels[NUM_CHANNELS];
unsigned char*	channelsend[NUM_CHANNELS];
int		channelstart[NUM_CHANNELS];
int 		channelhandles[NUM_CHANNELS];
int		channelids[NUM_CHANNELS];
int		steptable[256];
int		vol_lookup[128*256];
int*		channelleftvol_lookup[NUM_CHANNELS];
int*		channelrightvol_lookup[NUM_CHANNELS];

void I_SetChannels()
{
  printf("I_SetChannels\n");
}	

void I_SetSfxVolume(int volume)
{
  printf("I_SetSfxVolume: %d \n", volume);
}

int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
  char namebuf[9];
  sprintf(namebuf, "ds%s", sfx->name);
  return W_GetNumForName(namebuf);
}

int
I_StartSound
( int		id,
  int		vol,
  int		sep,
  int		pitch,
  int		priority )
{
  return 0;
}

void I_StopSound (int handle)
{
  printf("I_StopSound: %d \n", handle);
}

boolean I_SoundIsPlaying(int handle)
{
  printf("I_SoundIsPlaying: %d \n", handle);
  return 0;
}

void
I_UpdateSoundParams
( int	handle,
  int	vol,
  int	sep,
  int	pitch)
{
  //__builtin_trap();
}

void I_ShutdownSound(void)
{
  printf("I_ShutdownSound\n");
  //__builtin_trap();
}

void I_InitSound()
{
  printf("I_InitSound\n");
  //__builtin_trap();
}

void I_ShutdownMusic(void) 
{
  printf("I_ShutdownMusic\n");
  //__builtin_trap();
}

void I_InitMusic(void)
{
  printf("I_InitMusic\n");
  //__builtin_trap();
}

void I_PlaySong(int handle, int looping)
{
  doom_audio_play_song(handle, looping);
}

void I_PauseSong (int handle)
{
  printf("I_PauseSong handle: %d \n", handle);
  //__builtin_trap();
}

void I_ResumeSong (int handle)
{
  printf("I_ResumeSong handle: %d \n", handle);
  //__builtin_trap();
}

void I_StopSong(int handle)
{
  printf("I_StopSong handle: %d \n", handle);
  //__builtin_trap();
}

void I_UnRegisterSong(int handle)
{
  printf("I_UnRegisterSong handle: %d \n", handle);
  //__builtin_trap();
}

int I_RegisterSong(const void *data, size_t len)
{
  doom_audio_register_song((uint8_t *)data, len);

  return 0;
}

void I_SetMusicVolume(int volume)
{
  printf("I_SetMusicVolume volume: %d\n", volume);
  //__builtin_trap();
}
