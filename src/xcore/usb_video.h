#ifndef _usbv_h_
#define _usbv_h_

#include <xs1.h>
#include "ptr_buffers.h"
#include "lcd_defines.h"

/** \brief The USBV server thread.
 *
 * \param client The channel end connecting to the client.
 * \param ports The structure carrying the USBV port details.
 */
void usbv_server(client interface uint_ptr_rx rx,
                client interface uint_ptr_tx tx,
                chanend c_leds);


#endif
