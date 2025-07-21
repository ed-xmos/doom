#ifndef _usbv_h_
#define _usbv_h_

#include <xs1.h>
#include <stdint.h>
#include "ptr_buffers.h"
#include "doom_display.h"


#ifdef __XC__


/* Function to handle all endpoints of the Video class excluding control endpoint0 */
void VideoEndpointsHandler(chanend c_epint_in, chanend c_episo_in);

/* Endpoint 0 handles both std USB requests and Video class-specific requests */
void Endpoint0(chanend chan_ep0_out, chanend chan_ep0_in);

void usb_video_main(streaming chanend doom_usbv_display, streaming chanend c_lcd_trigger);

#endif // __XC__

#endif
