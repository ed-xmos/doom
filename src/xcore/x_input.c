#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <xcore/chanend.h>

#include "ps2.h"

port_t ps2_clock = XS1_PORT_1A; // found on J14 on the explorer
port_t ps2_data = XS1_PORT_1D;  // found on J14 on the explorer

void ps2_task(chanend_t c_ps2) {
    unsigned action, modifier, key;
    struct ps2state state;

    ps2HandlerInit(ps2_clock, &state);

	// Loop
    while (1) {
        ps2Handler(ps2_clock, ps2_data, 0, &state);
        ps2Interpret(&state, &action, &modifier, &key);
        if (action == PS2_PRESS || action == PS2_RELEASE) {
            chanend_out_byte(c_ps2, (unsigned char) action);
            chanend_out_byte(c_ps2, (unsigned char) modifier);
            chanend_out_byte(c_ps2, (unsigned char) key);
        }
    }
}
