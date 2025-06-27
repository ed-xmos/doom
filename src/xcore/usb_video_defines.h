#ifndef __USBV_DEFINES__
#define __USBV_DEFINES__

#define LCD_WIDTH            			 (320)
#define LCD_HEIGHT                       (200)
#define LCD_BITS_PER_PIXEL               (24)
#define LCD_HOR_FRONT_PORCH  			 (0)
#define LCD_HOR_BACK_PORCH               (0)
#define LCD_VERT_FRONT_PORCH             (0)
#define LCD_VERT_BACK_PORCH              (0)
#define LCD_HOR_PULSE_WIDTH              (0)
#define LCD_VERT_PULSE_WIDTH             (0)
#define LCD_FREQ_DIVIDEND                (100)
#define LCD_FREQ_DIVISOR                 (10)

#define LCD_ROW_WORDS (LCD_WIDTH*LCD_BITS_PER_PIXEL/32)

#endif
