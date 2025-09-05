#include <stdio.h>
#include <print.h>
#include <string.h>
#include <xs1.h>
#include <platform.h>
#include "spi.h"
#include <xcore/hwtimer.h>
#include <xcore/channel_streaming.h>
#include "doom_display.h"
#include "resource_adjust.h"

#define DC_PIN "DC_PIN"
#define CS_PIN "CS_PIN"
#define RST_PIN "RST_PIN"
#define LOW "LOW"
#define HIGH "HIGH"
#define OUTPUT "OUTPUT"



spi_master_device_t *g_dev;

#define RST_MASK (1 << 5)
#define DC_MASK (1 << 4)
unsigned p_dc_rst_state = 0; // RST low initially

port_t p_dc_rst = XS1_PORT_8D; // DC is bit 4, RST is bit 5

void delay(unsigned milliseconds){
  hwtimer_t tmr = hwtimer_alloc();
  hwtimer_delay(tmr, milliseconds * 100000);
  hwtimer_free(tmr);
}

void transfer(uint8_t data) {
  spi_master_start_transaction(g_dev);
  spi_master_transfer(g_dev, &data, 0, 1);
  spi_master_end_transaction(g_dev);
}


void tft_write_command(uint8_t cmd) {
  p_dc_rst_state &= ~DC_MASK;
  port_out(p_dc_rst, p_dc_rst_state);

  transfer(cmd);
}

void tft_write_data(uint8_t data) {
  p_dc_rst_state |= DC_MASK;
  port_out(p_dc_rst, p_dc_rst_state);

  transfer(data);
}

void tft_write_data16(uint16_t data) {
  p_dc_rst_state |= DC_MASK;
  port_out(p_dc_rst, p_dc_rst_state);

  transfer(data >> 8);
  transfer(data & 0xff);
}

void ili9341_init() {
  // Reset sequence
  p_dc_rst_state &= ~RST_MASK;
  port_out(p_dc_rst, p_dc_rst_state);
  delay(20);
  p_dc_rst_state |= RST_MASK;
  port_out(p_dc_rst, p_dc_rst_state);
  delay(150);

  // Exit sleep
  tft_write_command(0x11); // Sleep out
  delay(120);

  // Pixel format
  tft_write_command(0x3A); // COLMOD: Pixel Format Set
  tft_write_data(0x55);    // 16 bits/pixel

  // Memory Access Control - landscape
  tft_write_command(0x36); // MADCTL
  tft_write_data(0xE0); // Landscape: MX, bit 3 is 1->BGR 0->RGB 

/*
Portrait (0): MADCTL = 0x48
Landscape (1): MADCTL = 0x28
Portrait inverted (2): MADCTL = 0x88
Landscape inverted (3): MADCTL = 0xE8
*/

  // Display ON
  tft_write_command(0x29); // Display ON
  delay(20);

}


uint16_t framebuffer[LCD_WIDTH * LCD_HEIGHT] = {0};


void set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  tft_write_command(0x2A); // Column Address Set
  tft_write_data16(x0);
  tft_write_data16(x1);

  tft_write_command(0x2B); // Page Address Set
  tft_write_data16(y0);
  tft_write_data16(y1);

  tft_write_command(0x2C); // Memory Write
}

void copy_framebuffer_to_ili9341(const uint16_t* framebuffer) {
  // Set the full-screen address window
  set_address_window(0, 0, LCD_WIDTH  - 1, LCD_HEIGHT - 1);  // 320x240 area

  // Begin data transmission
  p_dc_rst_state |= DC_MASK;
  port_out(p_dc_rst, p_dc_rst_state);

  // Push all 153600 bytes over SPI
  int t1 = get_reference_time();
  spi_master_start_transaction(g_dev);
  spi_master_transfer(g_dev, (uint8_t*)framebuffer, 0, sizeof(uint16_t) * LCD_WIDTH * LCD_HEIGHT);
  spi_master_end_transaction(g_dev);
  int t2 = get_reference_time();
  (void)t1;
  (void)t2;
  // printintln(t2 - t1);
}

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((b & 0xF8) << 8) |   // top 5 bits of blue
           ((r & 0xFC) << 3) |   // top 6 bits of red
           (g >> 3);             // top 5 bits of green
}


void lcd(chanend_t c_lcd_trigger){
  port_enable(p_dc_rst);

  spi_master_t spi_mstr;
  spi_master_device_t spi_dev;
  g_dev = &spi_dev;

  // The ILI9341 supports SPI modes 0 and 3
  const int cpol = 0;
  const int cpha = 0;
  const int spi_div = 3; // 10 = 14MHz, 9 = 16MHz, 8 = 19MHz, 7 = 21MHz, 6 = 25MHz (20FPS), 5 = 30MHz, 4 = 38MHz, 3 = 50MHz (40FPS)
  
  port_t p_ss = XS1_PORT_1M;
  port_t p_clk = XS1_PORT_1O;
  port_t p_mosi = XS1_PORT_1P;

  spi_master_init(&spi_mstr, XS1_CLKBLK_1, p_ss, p_clk, p_mosi, 0/*miso*/);
  spi_master_device_init(&spi_dev, &spi_mstr, 0 /*cs pin*/, cpol, cpha, 
                        spi_master_source_clock_xcore, spi_div,
                        spi_master_sample_delay_0, 0, 0, 0, 0);

  // Had some problems with EMI!
  set_pad_output_slew(p_clk);
  set_pad_output_slew(p_mosi);
  set_pad_output_slew(p_ss);
  #define DRIVE_STRENGTH DRIVE_4MA // 2MA doesn't work at 50MHz. Lowest we can go.
  set_pad_drive_strength(p_clk, DRIVE_STRENGTH);
  set_pad_drive_strength(p_mosi, DRIVE_STRENGTH);
  set_pad_drive_strength(p_ss, DRIVE_STRENGTH);

  ili9341_init();

  while(1){
    copy_framebuffer_to_ili9341(framebuffer);
    s_chan_in_byte(c_lcd_trigger); // Hold off until we get a token
  }
}