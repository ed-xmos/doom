#include <platform.h>
#include <stdlib.h>
#include <stdio.h>
// #include "lcd.h"
#include "usb_video.h"
#include "doom_display.h"

// lcd_ports ports = {
//   XS1_PORT_1G, /* clk */
//   XS1_PORT_1F, /* de */
//   XS1_PORT_16A, /* data */
//   XS1_PORT_1B, /* hsync */
//   XS1_PORT_1C, /* vsync */
//   XS1_CLKBLK_1
// };

extern "C" {
int doom_main(int argc, char **argv);
}

static void doom_task(int argc, char * unsafe * unsafe argv,
               client interface doom_display display)
{
  client interface doom_display * movable p = &display;
  doom_display_set_pointer(move(p));
  int status = doom_main(argc, argv);

  printf("BREXIT\n");
  exit(status);
}

static void doom_task_fixed_args(client interface doom_display display)
{
  client interface doom_display * movable p = &display;
  doom_display_set_pointer(move(p));
  
  unsafe{
    int xargc; 
    const char * unsafe * unsafe xargv = {""};
    int status = doom_main(xargc, xargv);

    printf("BREXIT\n");
    exit(status);
  }
}

on tile[1]: out port p_leds = XS1_PORT_32A;

void led_task(chanend c_led){
  int cmd;
  while(1){
    select{
      case c_led :> cmd:
        p_leds <: cmd;
        break;
    }
  }
}



int main(void)
// int main(int argc, char * unsafe * unsafe argv)
{
  interface uint_ptr_tx_slave to_buffer;
  interface uint_ptr_rx to_lcd;
  interface uint_ptr_tx from_lcd;
  interface uint_ptr_rx from_buffer;
  interface doom_display display;

  chan c_led;

  par {
    on tile[0]:
    par {
      // doom_task(argc, argv, display);
      doom_task_fixed_args(display);
      uint_ptr_buffer_tx_slave(to_buffer, to_lcd);
      usbv_server(to_lcd, from_lcd, c_led);
      uint_ptr_buffer(from_lcd, from_buffer);
      doom_display(display, to_buffer, from_buffer);
    }
    on tile[1]:
    par{
      led_task(c_led);
      usb_video_main();
    }
  }
  return 0;
}
