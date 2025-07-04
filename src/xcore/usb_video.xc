#include "usb_video.h"
#include <xs1.h>
#include <stdio.h>

uint8_t yuv2_frame[2 * SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
unsafe{ unsigned int *unsafe img_ptr = (unsigned int *)yuv2_frame;
        uint8_t *unsafe yuv2_frame_ptr = yuv2_frame;
}



static void
output_row(unsigned short row[LCD_ROW_WORDS * 2],
           unsigned &time)
{
  static int r = 0; 
  if(++r == 200 * 100){
    printf("Display 100 frames\n");
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

    time += XS1_TIMER_HZ / 30;

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


#include "string.h"
#include "stdlib.h"

#include "usb_video.h"
#include "xud_device.h"
#include "uvc_req.h"
#include "uvc_defs.h"

// #include "img.h"

/* Definition of Descriptors */
/* USB Device Descriptor */
static unsigned char devDesc[] =
{
    0x12,                  /* 0  bLength */
    USB_DESCTYPE_DEVICE,   /* 1  bdescriptorType - Device*/
    0x00,                  /* 2  bcdUSB version */
    0x02,                  /* 3  bcdUSB version */
    0xEF,                  /* 4  bDeviceClass - USB Miscellaneous Class */
    0x02,                  /* 5  bDeviceSubClass  - Common Class */
    0x01,                  /* 6  bDeviceProtocol  - Interface Association Descriptor */
    0x40,                  /* 7  bMaxPacketSize for EP0 - max = 64*/
    (VENDOR_ID & 0xFF),    /* 8  idVendor */
    (VENDOR_ID >> 8),      /* 9  idVendor */
    (PRODUCT_ID & 0xFF),   /* 10 idProduct */
    (PRODUCT_ID >> 8),     /* 11 idProduct */
    (BCD_DEVICE & 0xFF),   /* 12 bcdDevice */
    (BCD_DEVICE >> 8),     /* 13 bcdDevice */
    0x01,                  /* 14 iManufacturer - index of string*/
    0x02,                  /* 15 iProduct  - index of string*/
    0x00,                  /* 16 iSerialNumber  - index of string*/
    0x01                   /* 17 bNumConfigurations */
};

/* USB Configuration Descriptor */
static unsigned char cfgDesc[] = {

  0x09,                       /* 0  bLength */
  USB_DESCTYPE_CONFIGURATION, /* 1  bDescriptorType - Configuration*/
  0xAE,00,                    /* 2  wTotalLength */
  0x02,                       /* 4  bNumInterfaces */
  0x01,                       /* 5  bConfigurationValue */
  0x03,                       /* 6  iConfiguration - index of string */
  0x80,                       /* 7  bmAttributes - Bus powered */
  0xFA,                       /* 8  bMaxPower (in 2mA units) - 500mA */

  /* Interface Association Descriptor */
  0x08,                       /* 0 bLength */
  USB_DESCTYPE_INTERFACE_ASSOCIATION,  /* 1 bDescriptorType - Interface Association */
  0x00,                       /* 2 bFirstInterface - VideoControl i/f */
  0x02,                       /* 3 bInterfaceCount - 2 Interfaces */
  USB_CLASS_VIDEO,            /* 4 bFunctionClass - Video Class */
  USB_VIDEO_INTERFACE_COLLECTION,  /* 5 bFunctionSubClass - Video Interface Collection */
  0x00,                       /* 6 bFunctionProtocol - No protocol */
  0x02,                       /* 7 iFunction - index of string */

  /* Video Control (VC) Interface Descriptor */
  0x09,                       /* 0 bLength */
  USB_DESCTYPE_INTERFACE,     /* 1 bDescriptorType - Interface */
  0x00,                       /* 2 bInterfaceNumber - Interface 0 */
  0x00,                       /* 3 bAlternateSetting */
  0x01,                       /* 4 bNumEndpoints */
  USB_CLASS_VIDEO,            /* 5 bInterfaceClass - Video Class */
  USB_VIDEO_CONTROL,          /* 6 bInterfaceSubClass - VideoControl Interface */
  0x00,                       /* 7 bInterfaceProtocol - No protocol */
  0x02,                       /* 8 iInterface - Index of string (same as iFunction of IAD) */

  /* Class-specific VC Interface Header Descriptor */
  0x0D,                       /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,  /* 1 bDescriptorType - Class-specific Interface */
  USB_VC_HEADER,              /* 2 bDescriptorSubType - HEADER */
  0x10, 0x01,                 /* 3 bcdUVC - Video class revision 1.1 */
  0x28, 0x00,                 /* 5 wTotalLength - till output terminal */
  WORD_CHARS(100000000),      /* 7 dwClockFrequency - 100MHz (Deprecated) */
  0x01,                       /* 11 bInCollection - One Streaming Interface */
  0x01,                       /* 12 baInterfaceNr - Number of the Streaming interface */

  /* Input Terminal (Camera) Descriptor - Represents the CCD sensor (Simulated here in this demo) */
  0x12,                       /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,  /* 1 bDescriptorType - Class-specific Interface */
  USB_VC_INPUT_TERMINAL,      /* 2 bDescriptorSubType - INPUT TERMINAL */
  0x01,                       /* 3 bTerminalID */
  0x01, 0x02,                 /* 4 wTerminalType - ITT_CAMERA type (CCD Sensor) */
  0x00,                       /* 6 bAssocTerminal - No association */
  0x00,                       /* 7 iTerminal - Unused */
  0x00, 0x00,                 /* 8 wObjectiveFocalLengthMin - No optical zoom supported */
  0x00, 0x00,                 /* 10 wObjectiveFocalLengthMax - No optical zoom supported*/
  0x00, 0x00,                 /* 12 wOcularFocalLength - No optical zoom supported */
  0x03,                       /* 14 bControlSize - 3 bytes */
  0x00, 0x00, 0x00,           /* 15 bmControls - No controls are supported */

  /* Output Terminal Descriptor */
  0x09,                       /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,  /* 1 bDescriptorType - Class-specific Interface */
  USB_VC_OUPUT_TERMINAL,      /* 2 bDescriptorSubType - OUTPUT TERMINAL  */
  0x02,                       /* 3 bTerminalID */
  0x01, 0x01,                 /* 4 wTerminalType - TT_STREAMING type */
  0x00,                       /* 6 bAssocTerminal - No association */
  0x01,                       /* 7 bSourceID - Source is Input terminal 1 */
  0x00,                       /* 8 iTerminal - Unused */

  /* Standard Interrupt Endpoint Descriptor */
  0x07,                       /* 0 bLength */
  USB_DESCTYPE_ENDPOINT,      /* 1 bDescriptorType */
  (VIDEO_STATUS_EP_NUM | 0x80), /* 2 bEndpointAddress - IN endpoint*/
  0x03,                       /* 3 bmAttributes - Interrupt transfer */
  0x40, 0x00,                 /* 4 wMaxPacketSize - 64 bytes */
  0x09,                       /* 6 bInterval - 2^(9-1) microframes = 32ms */

  /* Class-specific Interrupt Endpoint Descriptor */
  0x05,                       /* 0 bLength */
  USB_DESCTYPE_CS_ENDPOINT,   /* 1 bDescriptorType - Class-specific Endpoint */
  0x03,                       /* 2 bDescriptorSubType - Interrupt Endpoint */
  0x40, 0x00,                 /* 3 wMaxTransferSize - 64 bytes */

  /* Video Streaming Interface Descriptor */
  /* Zero-bandwidth Alternate Setting 0 */
  0x09,                       /* 0 bLength */
  USB_DESCTYPE_INTERFACE,     /* 1 bDescriptorType - Interface */
  0x01,                       /* 2 bInterfaceNumber - Interface 1 */
  0x00,                       /* 3 bAlternateSetting - 0 */
  0x00,                       /* 4 bNumEndpoints - No bandwidth used */
  USB_CLASS_VIDEO,            /* 5 bInterfaceClass - Video Class */
  USB_VIDEO_STREAMING,        /* 6 bInterfaceSubClass - VideoStreaming Interface */
  0x00,                       /* 7 bInterfaceProtocol - No protocol */
  0x00,                       /* 8 iInterface - Unused */

  /* Class-specific VS Interface Input Header Descriptor */
  0x0E,                       /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,  /* 1 bDescriptorType - Class-specific Interface */
  USB_VS_INPUT_HEADER,        /* 2 bDescriptorSubType - INPUT HEADER */
  0x01,                       /* 3 bNumFormats - One format supported */
  0x47, 0x00,                 /* 4 wTotalLength - Size of class-specific VS descriptors */
  (VIDEO_DATA_EP_NUM | 0x80), /* 6 bEndpointAddress - Iso EP for video streaming */
  0x00,                       /* 7 bmInfo - No dynamic format change */
  0x02,                       /* 8 bTerminalLink - Denotes the Output Terminal */
  0x01,                       /* 9 bStillCaptureMethod - Method 1 supported */
  0x00,                       /* 10 bTriggerSupport - No Hardware Trigger */
  0x00,                       /* 11 bTriggerUsage */
  0x01,                       /* 12 bControlSize - 1 byte */
  0x00,                       /* 13 bmaControls - No Controls supported */

  /* Class-specific VS Format Descriptor */
  0x1B,                       /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,  /* 1 bDescriptorType - Class-specific Interface */
  USB_VS_FORMAT_UNCOMPRESSED, /* 2 bDescriptorSubType - FORMAT UNCOMPRESSED */
  0x01,                       /* 3 bFormatIndex */
  0x01,                       /* 4 bNumFrameDescriptors - 1 Frame descriptor followed */
  0x59,0x55,0x59,0x32,
  0x00,0x00,0x10,0x00,
  0x80,0x00,0x00,0xAA,
  0x00,0x38,0x9B,0x71,        /* 5 guidFormat - YUY2 Video format */
  BITS_PER_PIXEL,             /* 21 bBitsPerPixel - 16 bits */
  0x01,                       /* 22 bDefaultFrameIndex */
  0x00,                       /* 23 bAspectRatioX */
  0x00,                       /* 24 bAspectRatioY */
  0x00,                       /* 25 bmInterlaceFlags - No interlaced mode */
  0x00,                       /* 26 bCopyProtect - No restrictions on duplication */

  /* Class-specific VS Frame Descriptor */
  0x1E,                       /* 0 bLength */
  USB_DESCTYPE_CS_INTERFACE,  /* 1 bDescriptorType - Class-specific Interface */
  USB_VS_FRAME_UNCOMPRESSED,  /* 2 bDescriptorSubType */
  0x01,                       /* 3 bFrameIndex */
  0x01,                       /* 4 bmCapabilities - Still image capture method 1 */
  SHORT_CHARS(WIDTH),         /* 5 wWidth */
  SHORT_CHARS(HEIGHT),        /* 7 wHeight */
  WORD_CHARS(MIN_BIT_RATE),   /* 9 dwMinBitRate */
  WORD_CHARS(MAX_BIT_RATE),   /* 13 dwMaxBitRate */
  WORD_CHARS(MAX_FRAME_SIZE), /* 17 dwMaxVideoFrameBufSize */
  WORD_CHARS(FRAME_INTERVAL), /* 21 dwDefaultFrameInterval (in 100ns units) */
  0x01,                       /* 25 bFrameIntervalType */
  WORD_CHARS(FRAME_INTERVAL), /* 26 dwFrameInterval (in 100ns units) */

  /* Video Streaming Interface Descriptor */
  /* Alternate Setting 1 */
  0x09,                       /* 0 bLength */
  USB_DESCTYPE_INTERFACE,     /* 1 bDescriptorType - Interface */
  0x01,                       /* 2 bInterfaceNumber - Interface 1 */
  0x01,                       /* 3 bAlternateSetting -  1 */
  0x01,                       /* 4 bNumEndpoints */
  USB_CLASS_VIDEO,            /* 5 bInterfaceClass - Video Class */
  USB_VIDEO_STREAMING,        /* 6 bInterfaceSubClass - VideoStreaming Interface */
  0x00,                       /* 7 bInterfaceProtocol - No protocol */
  0x00,                       /* 8 iInterface - Unused */

  /* Standard VS Isochronous Video Data Endpoint Descriptor */
  0x07,                       /* 0 bLength */
  USB_DESCTYPE_ENDPOINT,      /* 1 bDescriptorType */
  (VIDEO_DATA_EP_NUM | 0x80), /* 2 bEndpointAddress - IN Endpoint */
  0x05,                       /* 3 bmAttributes - Isochronous EP (Asynchronous) */
  0x00, 0x04,                 /* 4 wMaxPacketSize 1x 1024 bytes*/
  0x01,                       /* 6 bInterval */
};

unsafe{
  /* String table - unsafe as accessed via shared memory */
  static char * unsafe stringDescriptors[]=
  {
    "\x09\x04",             /* Language ID string (US English) */
    "XMOS",                 /* iManufacturer */
    "XMOS USB Video Device",/* iProduct */
    "Config",               /* iConfiguration string */
  };
}

/* Endpoint 0 handles both std USB requests and Video class-specific requests */
void Endpoint0(chanend chan_ep0_out, chanend chan_ep0_in)
{
    USB_SetupPacket_t sp;

    unsigned bmRequestType;
    XUD_BusSpeed_t usbBusSpeed;

    XUD_ep ep0_out = XUD_InitEp(chan_ep0_out);
    XUD_ep ep0_in = XUD_InitEp(chan_ep0_in);

    UVC_InitProbeCommitData();

    while(1)
    {
       /* Returns XUD_RES_OKAY on success */
       XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, sp);

       if(result == XUD_RES_OKAY)
       {
           /* Set result to ERR, we expect it to get set to OKAY if a request is handled */
           result = XUD_RES_ERR;

           /* Stick bmRequest type back together for an easier parse... */
           bmRequestType = (sp.bmRequestType.Direction<<7) |
                           (sp.bmRequestType.Type<<5) |
                           (sp.bmRequestType.Recipient);

           if ((bmRequestType == USB_BMREQ_H2D_STANDARD_DEV) &&
               (sp.bRequest == USB_SET_ADDRESS))
           {
             // Host has set device address, value contained in sp.wValue
           }

           switch(bmRequestType)
           {
               /* Direction: Device-to-host and Host-to-device
                * Type: Class
                * Recipient: Interface / Endpoint
                */
               case USB_BMREQ_H2D_CLASS_INT:
               case USB_BMREQ_D2H_CLASS_INT:
               case USB_BMREQ_H2D_CLASS_EP:
               case USB_BMREQ_D2H_CLASS_EP:

                   /* Inspect for VideoControl Class interface number or
                    * VideoStreaming Class interface number or EP number;
                    * If an Entity is addressed, the High byte has to be checked
                    * for Entity ID */
                   if(sp.wIndex == 0 || sp.wIndex == 1 || sp.wIndex == (VIDEO_DATA_EP_NUM | 0x80))
                   {
                       /* Returns  XUD_RES_OKAY if handled,
                        *          XUD_RES_ERR if not handled,
                        *          XUD_RES_RST for bus reset */
                       result = UVC_InterfaceClassRequests(ep0_out, ep0_in, sp);
                   }
                   break;
           }
       } /* if ends */

       /* If we haven't handled the request about then do standard enumeration requests */
       if(result == XUD_RES_ERR )
       {
           /* Returns  XUD_RES_OKAY if handled okay,
            *          XUD_RES_ERR if request was not handled (STALLed),
            *          XUD_RES_RST for USB Reset */
           unsafe{
           result = USB_StandardRequests(ep0_out, ep0_in, devDesc,
                       sizeof(devDesc), cfgDesc, sizeof(cfgDesc),
                       null, 0, null, 0, stringDescriptors, sizeof(stringDescriptors)/sizeof(stringDescriptors[0]),
                       sp, usbBusSpeed);
            }
       }

       /* USB bus reset detected, reset EP and get new bus speed */
       if(result == XUD_RES_RST)
       {
           usbBusSpeed = XUD_ResetEndpoint(ep0_out, ep0_in);
       }
    }
}

/* Function to handle all endpoints of the Video class excluding control endpoint0 */
void VideoEndpointsHandler(chanend c_epint_in, chanend c_episo_in)
{
    /* Buffer to hold Video data in YUYV format */
    unsigned int gVideoBuffer[PAYLOAD_SIZE / sizeof(int)];
    uint8_t *payload_header_ptr = (uint8_t*)&gVideoBuffer[0];
    unsigned int *payload_data_ptr = &gVideoBuffer[HEADER_BYTES/sizeof(int)];
    // unsigned int *img_ptr = (unsigned int *)img_yuv;
    unsigned int line_count = 0;
    int frame = 0x0C;
    int pts, tmrValue = 0;

    XUD_Result_t result;
    timer presentationTimer;

    int sofCounts = 0;
    int increment = 0;

    /* Initialize all endpoints */
    XUD_ep epint_in = XUD_InitEp(c_epint_in);
    XUD_ep episo_in = XUD_InitEp(c_episo_in);

    /* Just to keep compiler happy */
    epint_in = epint_in;
    /* XUD will NAK if the endpoint is not ready to communicate with XUD */

    /* Fill video buffers with different color data */
    printstr("Filling video buffer with color data\n");
    while(1)
    {
        int expectedPixels =  MAX_FRAME_SIZE;
        presentationTimer :> pts;

        /* Fill the payload buffer with payload header */
        /* Make the Payload header */
        payload_header_ptr[0] = HEADER_BYTES;
        payload_header_ptr[1] = frame;
        memcpy(&payload_header_ptr[2], &pts, sizeof(pts));
        memcpy(&payload_header_ptr[6], &pts, sizeof(pts));
        short sof_count_short = (sofCounts>>3) & 2047;
        memcpy(&payload_header_ptr[10], &sof_count_short, sizeof(sof_count_short));

        /* Fill the payload buffer with payload data (one line of the image) */
        unsigned start_of_line = line_count * (LINE_SIZE_BYTES / sizeof(int));
        unsigned middle_of_line = start_of_line + increment;
        unsigned line_size = LINE_SIZE_BYTES / sizeof(int);

        // Ensure increment does not exceed line_size
        if (increment > line_size) {
            increment = line_size;
        }

        // Copy data from middle_of_line to the end of the line
        for (unsigned i = 0; i < line_size - increment; i++) unsafe{
            payload_data_ptr[i] = img_ptr[middle_of_line++];
        }

        // Copy data from start_of_line to increment
        for (unsigned j = 0; j < increment; j++) unsafe{
            payload_data_ptr[line_size - increment + j] = img_ptr[start_of_line++];
        }

        line_count += 1;
        if(line_count >= HEIGHT)
        {
            line_count = 0;
            frame = frame ^ 1;  /* Toggle FID bit */
            sofCounts += HEIGHT;
            // increment++; // Don't roll display
            if (increment >= LINE_SIZE_BYTES / sizeof(int)) {
                increment = 0;
            }
        }

        presentationTimer :> tmrValue;

        /* Payload transfer */
        result = XUD_SetBuffer(episo_in, (gVideoBuffer, unsigned char[]), PAYLOAD_SIZE);

    }
}


// Helper function to convert to YUV
// Converts two BGR565 pixels to one YUV422 (YUYV) 4-byte block
void bgr565_pair_to_yuyv(uint16_t p0, uint16_t p1, uint8_t *dst) {
    // Unpack first pixel
    uint8_t b0 = ((p0 >> 11) & 0x1F) << 3;
    uint8_t g0 = ((p0 >> 5) & 0x3F) << 2;
    uint8_t r0 = (p0 & 0x1F) << 3;
    b0 |= b0 >> 5; g0 |= g0 >> 6; r0 |= r0 >> 5;

    // Unpack second pixel
    uint8_t b1 = ((p1 >> 11) & 0x1F) << 3;
    uint8_t g1 = ((p1 >> 5) & 0x3F) << 2;
    uint8_t r1 = (p1 & 0x1F) << 3;
    b1 |= b1 >> 5; g1 |= g1 >> 6; r1 |= r1 >> 5;

    // Convert to YUV using integer BT.601
    uint8_t y0 = (uint8_t)(((66 * r0 + 129 * g0 + 25 * b0 + 128) >> 8) + 16);
    uint8_t y1 = (uint8_t)(((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16);

    uint8_t u0 = (uint8_t)(((-38 * r0 - 74 * g0 + 112 * b0 + 128) >> 8) + 128);
    uint8_t u1 = (uint8_t)(((-38 * r1 - 74 * g1 + 112 * b1 + 128) >> 8) + 128);
    uint8_t v0 = (uint8_t)(((112 * r0 - 94 * g0 - 18 * b0 + 128) >> 8) + 128);
    uint8_t v1 = (uint8_t)(((112 * r1 - 94 * g1 - 18 * b1 + 128) >> 8) + 128);

    // Average U and V
    uint8_t u = (u0 + u1) >> 1;
    uint8_t v = (v0 + v1) >> 1;

    // Output: Y0 U Y1 V
    dst[0] = y0;
    dst[1] = u;
    dst[2] = y1;
    dst[3] = v;
}

void buffer_rx(server interface doom_usbv_display_t i_doom_usbv_display){
    uint16_t palette[256] = {0};
    uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};

    int convert_frame = 0;

    timer t;
    int frames_counted = 0;
    int time_then;
    t :> time_then;

    while(1){
        select{
            case i_doom_usbv_display.set_palette(const uint16_t new_palette[256]):
                memcpy(palette, new_palette, sizeof(new_palette));
                // printf("buffer_rx set_palette\n");
                // convert_frame = 1;
                break;
            case i_doom_usbv_display.write_frame(const uint8_t new_frame[SCREEN_WIDTH * SCREEN_HEIGHT]):
                memcpy(frame, new_frame, sizeof(new_frame));
                // printf("buffer_rx write_frame\n");
                convert_frame = 1;
                break;

            convert_frame => default:
                frames_counted++;
                int time_now;
                t :> time_now;
                if(timeafter(time_now, time_then + XS1_TIMER_HZ)){
                    printf("FPS: %d\n", frames_counted);
                    time_then += XS1_TIMER_HZ;
                    frames_counted = 0;
                }
                convert_frame = 0;
                // Convert 2 pix at a time
                for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i += 2)unsafe{
                    uint8_t *yuv2_ptr;
                    unsafe{yuv2_ptr = (uint8_t*)&yuv2_frame_ptr[i * 2];}
                    uint16_t rgb1 = palette[frame[i]];
                    uint16_t rgb2 = palette[frame[i+1]];

                    bgr565_pair_to_yuyv(rgb1, rgb2, yuv2_ptr);
                }
                break;
        }
    }
}


/* Endpoint count defines */
#define EP_COUNT_OUT   1    // 1 OUT EP0
#define EP_COUNT_IN    3    // (1 IN EP0 + 1 INTERRUPT IN EP + 1 ISO IN EP)

/* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
 * if the endpoint wishes to be informed of USB bus resets
 */
XUD_EpType epTypeTableOut[EP_COUNT_OUT] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE};
XUD_EpType epTypeTableIn[EP_COUNT_IN] =   {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_INT, XUD_EPTYPE_ISO};


void usb_video_main(server interface doom_usbv_display_t i_doom_usbv_display) {

    chan c_ep_out[EP_COUNT_OUT], c_ep_in[EP_COUNT_IN];

    printf("GO USB!\n");

    /* 'Par' statement to run the following tasks in parallel */
    par
    {
        XUD_Main(c_ep_out, EP_COUNT_OUT, c_ep_in, EP_COUNT_IN,
                      null, epTypeTableOut, epTypeTableIn,
                      XUD_SPEED_HS, XUD_PWR_BUS);

        Endpoint0(c_ep_out[0], c_ep_in[0]);

        VideoEndpointsHandler(c_ep_in[1], c_ep_in[2]);

        buffer_rx(i_doom_usbv_display);
    }
}
