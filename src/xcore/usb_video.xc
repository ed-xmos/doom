#include "usb_video.h"
#include <xs1.h>
#include <stdio.h>

static void
output_row(unsigned short row[LCD_ROW_WORDS * 2],
           unsigned &time)
{
  static int r = 0; 
  if(++r == 200){
    printf("Display frame\n");
    r = 0;
  }
}

void usbv_server(client interface uint_ptr_rx rx,
                client interface uint_ptr_tx tx,
                chanend c_leds) {

  printf("usbv_server\n");

  unsigned * movable ptr = rx.pop();

  unsigned time = 100;

  while (1) {

    time += LCD_HSYNC_TIME * (LCD_VERT_BACK_PORCH - LCD_VERT_PULSE_WIDTH);

    for (int y = 0; y < LCD_HEIGHT; y++) {
      // partout_timed(p.lcd_hsync, LCD_HOR_PULSE_WIDTH + 1,
      //               1 << LCD_HOR_PULSE_WIDTH, time);
      time += LCD_HOR_BACK_PORCH;

      if (!ptr)
        ptr = rx.pop();
      output_row((unsigned short * movable)ptr, time);
      tx.push(move(ptr));
      time += LCD_HOR_FRONT_PORCH;
    }
  }
}
