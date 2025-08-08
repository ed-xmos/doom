#include "doom_display.h"
// #include "lcd.h"
#include "usb_video.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <xclib.h>


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

extern uint16_t framebuffer[LCD_WIDTH * LCD_HEIGHT];

static inline uint16_t byte_swap_16b(uint16_t bgr){
    // This works for bgr mode
    bgr = byterev((uint32_t)bgr) >> 16;

    return bgr;
}

void lcd_renderer(streaming chanend doom_usbv_display, streaming chanend c_lcd_trigger){
    uint16_t palette[256] = {0};
    uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};

    timer t;
    int frames_counted = 0;
    int time_then;
    t :> time_then;

    while(1){
        select{
            case doom_usbv_display :> int cmd:
                switch(cmd){
                    case DD_SET_PALETTE:
                        sin_char_array(doom_usbv_display, (char *)palette, sizeof(palette));
                        break;
                    case DD_WRITE:
                        sin_char_array(doom_usbv_display, frame, sizeof(frame));
                        frames_counted++;
                        int time_now;
                        t :> time_now;
                        if(timeafter(time_now, time_then + XS1_TIMER_HZ)){
                            printf("FPS: %d\n", frames_counted);
                            time_then += XS1_TIMER_HZ;
                            frames_counted = 0;
                        }
                        // Convert 2 pix at a time
                        for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i += 2)unsafe{
                            uint16_t rgb1 = palette[frame[i]];
                            uint16_t rgb2 = palette[frame[i+1]];

                            const int offset = LCD_WIDTH * VERTICAL_OFFSET;
                            // Populate LCD buffer this is picked up via shared mem
                            framebuffer[i + offset ] = byte_swap_16b(palette[frame[i]]);
                            framebuffer[i + offset + 1] = byte_swap_16b(palette[frame[i + 1]]);
                        }
                        // Trigger LCD refresh
                        c_lcd_trigger <: (char)0;
                        break;
                } //switch
                break; // case
        } //select
    }
}
