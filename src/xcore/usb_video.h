#ifndef _usbv_h_
#define _usbv_h_

#include <xs1.h>
#include <stdint.h>
#include "ptr_buffers.h"
#include "doom_display.h"


#ifdef __XC__

/** \brief The USBV server thread.
 *
 * \param client The channel end connecting to the client.
 * \param ports The structure carrying the USBV port details.
 */
void usbv_server(client interface uint_ptr_rx rx,
                client interface uint_ptr_tx tx,
                chanend c_leds);


/* Function to handle all endpoints of the Video class excluding control endpoint0 */
void VideoEndpointsHandler(chanend c_epint_in, chanend c_episo_in);

/* Endpoint 0 handles both std USB requests and Video class-specific requests */
void Endpoint0(chanend chan_ep0_out, chanend chan_ep0_in);

void usb_video_main(streaming chanend doom_usbv_display);

#endif // __XC__

#endif
