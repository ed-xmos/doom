// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define APP_I2S_FREQUENCY               22050
#define APP_MCLK_FREQUENCY              (APP_I2S_FREQUENCY * 1024)
#define APP_NUM_I2S_CHANNELS_OUT        2
#define APP_NUM_I2S_LINES_OUT           ((APP_NUM_I2S_CHANNELS_OUT + 1) / 2)
#define APP_MIDI_NUM_CHANNELS           APP_NUM_I2S_CHANNELS_OUT
#define APP_MIDI_SAMPLE_RATE			APP_I2S_FREQUENCY
#define MAX_MIDI_TRACK_SIZE				30000
#define NUM_MIDI_TRACKS 				2