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

//FFS
#include "filesystem.h"
#include "qspi_flash_storage_media.h"
#include <quadflash.h>
#include <QuadSpecMacros.h>

fl_QSPIPorts qspi_flash_ports = {
  PORT_SQI_CS,
  PORT_SQI_SCLK,
  PORT_SQI_SIO,
  on tile[0]: XS1_CLKBLK_1
};



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

unsafe client interface fs_basic_if g_i_fs;

static void doom_task_fixed_args(client interface doom_display display, client interface fs_basic_if i_fs)
{
  client interface doom_display * movable p = &display;
  doom_display_set_pointer(move(p));

  int result = 0;
  printf("Mounting filesystem...\n");
  result = i_fs.mount();
  printf("result = %d\n", result);

  printf("Opening file...\n");
  char filename[] = "DOOM1.WAD";
  result = i_fs.open(filename, sizeof(filename));
  printf("result = %d\n", result);
  
  printf("Getting file size...\n");
  size_t file_size = 0;
  result = i_fs.size(file_size);
  printf("size = %d result = %d\n", file_size, result);
  
  unsafe{g_i_fs = i_fs;}
  
  unsafe{
    int xargc; 
    const char * unsafe * unsafe xargv = {""};
    int status = doom_main(xargc, xargv);

    printf("BREXIT\n");
    exit(status);
  }
}

on tile[1]: out port p_leds = XS1_PORT_32A;


int main(void)
// int main(int argc, char * unsafe * unsafe argv)
{
  interface uint_ptr_tx_slave to_buffer;
  interface uint_ptr_rx to_lcd;
  interface uint_ptr_tx from_lcd;
  interface uint_ptr_rx from_buffer;
  interface doom_display display;

  interface doom_usbv_display_t i_doom_usbv_display;

  interface fs_basic_if i_fs[1];
  interface fs_storage_media_if i_media;

  par {
    on tile[0]:
    par {
      // doom_task(argc, argv, display);
      doom_task_fixed_args(display, i_fs[0]);
      // uint_ptr_buffer_tx_slave(to_buffer, to_lcd);
      // usbv_server(to_lcd, from_lcd, c_led);
      // uint_ptr_buffer(from_lcd, from_buffer);
      doom_display(display, to_buffer, from_buffer, i_doom_usbv_display);

      {
        fl_QuadDeviceSpec qspi_spec = FL_QUADDEVICE_DEFAULT;
        qspi_flash_fs_media(i_media, qspi_flash_ports, qspi_spec, 512);
      }
      filesystem_basic(i_fs, 1, FS_FORMAT_FAT12, i_media);
    }
    on tile[1]:
    par{
      usb_video_main(i_doom_usbv_display);
    }
  }
  return 0;
}
