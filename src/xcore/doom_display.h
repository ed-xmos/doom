#ifndef _doom_display_h_
#define _doom_display_h_

#include <stdint.h>
#include "usb_video.h"

// Doom settings
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_BPP 8

// Output settings
#define LCD_WIDTH   320
#define LCD_HEIGHT  240

#define DD_SET_PALETTE 0
#define DD_WRITE       1

#define HORIZONTAL_OFFSET ((LCD_WIDTH - SCREEN_WIDTH) / 2)
#define VERTICAL_OFFSET ((LCD_HEIGHT - SCREEN_HEIGHT) / 2)

#ifdef __cplusplus
extern "C" {
#endif
void doom_display_set_palette(const uint16_t new_palette[256]);
void doom_display_write(const uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT]);
void doom_display_read(uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT]);
#ifdef __cplusplus
}
#endif

#endif //_doom_display_h_
