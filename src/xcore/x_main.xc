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
#include "pff.h"
#include "xk_evk_xu316/board.h"

fl_QSPIPorts qspi_flash_ports = {
  PORT_SQI_CS,
  PORT_SQI_SCLK,
  PORT_SQI_SIO,
  on tile[0]: XS1_CLKBLK_1
};


extern "C" {
  int doom_main(int argc, char **argv);
  void lcd(streaming chanend c_lcd_trigger);
  void audio_subsystem(chanend c_i2c, streaming chanend c_audio);
}
extern void lcd_renderer(streaming chanend doom_usbv_display, streaming chanend c_lcd_trigger);


// static void doom_task(int argc, char * unsafe * unsafe argv,
//                client interface doom_display display)
// {
//   client interface doom_display * movable p = &display;
//   doom_display_set_pointer(move(p));
//   int status = doom_main(argc, argv);

//   printf("BREXIT\n");
//   exit(status);
// }



unsafe client interface fs_basic_if g_i_fs;
FATFS fatfs;
unsafe streaming chanend g_doom_usbv_display;
unsafe streaming chanend g_c_audio;


static void doom_task_fixed_args( streaming chanend doom_usbv_display,
                                  streaming chanend c_audio)
{
  unsafe{
    g_doom_usbv_display = doom_usbv_display;
    g_c_audio = c_audio;
  }

  int result = 0;
  printf("Mounting filesystem...\n");
  // result = i_fs.mount();
  result = pf_mount(&fatfs);
  printf("result = %d\n", result);
  
  unsafe{
    int xargc; 
    const char * unsafe * unsafe xargv = {""};
    int status = doom_main(xargc, xargv);

    printf("BREXIT\n");
    exit(status);
  }
}


int main(void)
// int main(int argc, char * unsafe * unsafe argv)
{
  streaming chan doom_usbv_display; // From game to USB video class
  streaming chan c_lcd_trigger;
  streaming chan c_audio;
  chan c_i2c;

  interface fs_basic_if i_fs[1];
  interface fs_storage_media_if i_media;

  par {
    on tile[0]:
    par {
      // doom_task(argc, argv, display);
      doom_task_fixed_args(doom_usbv_display, c_audio);
      {
        fl_QuadDeviceSpec qspi_spec = FL_QUADDEVICE_DEFAULT;
        qspi_flash_fs_media(i_media, qspi_flash_ports, qspi_spec, 512);
      }
      filesystem_basic(i_fs, 1, FS_FORMAT_FAT12, i_media);
      // xk_evk_xu316_AudioHwRemote(c_i2c); // Startup remote I2C master server task

    }
    on tile[1]:
    par{
      lcd_renderer(doom_usbv_display, c_lcd_trigger);
      // usb_video_main(doom_usbv_display, c_lcd_trigger);
      lcd(c_lcd_trigger);
      audio_subsystem(c_i2c, c_audio);
    }
  }
  return 0;
}
