#include "doom_display.h"
#include "ptr_buffers.h"
// #include "lcd.h"
#include "usb_video.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>


extern unsafe streaming chanend g_doom_usbv_display;


void doom_display_set_palette(const uint16_t new_palette[256]){
  // printf("doom_display_set_palette\n");
  unsafe{
    g_doom_usbv_display <: (int)DD_SET_PALETTE;
    sout_char_array((streaming chanend)g_doom_usbv_display, (char *)new_palette, sizeof(new_palette));
  }
}

void doom_display_write(const uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT]){
  // printf("doom_display_write\n");
  unsafe{
    g_doom_usbv_display <: (int)DD_WRITE;
    sout_char_array((streaming chanend)g_doom_usbv_display, frame, sizeof(frame));
  }
}

void doom_display_read(uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT]){
  printf("*****doom_display_read UNSUPPORTED\n");
}