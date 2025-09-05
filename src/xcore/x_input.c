#include <xs1.h>
#include <platform.h>
#include <stdio.h>
#include <xcore/chanend.h>

#include "ps2.h"
#include "resource_adjust.h"
#include "doomdef.h"

port_t ps2_clock = XS1_PORT_1A; // found on J14 on the explorer
port_t ps2_data = XS1_PORT_1D;  // found on J14 on the explorer

extern int key_menu_right;
extern int key_menu_down;
extern int key_menu_left;
extern int key_menu_up;
extern int key_menu_enter;
extern int key_menu_escape;
extern int key_right;
extern int key_left;
extern int key_up;
extern int key_down;
extern int key_fire;
extern int key_use;
extern int key_strafe;
extern int key_speed;
extern int key_screenshot;

void ps2_task(chanend_t c_ps2) {
    unsigned action, modifier, key;
    struct ps2state state;

    // Hack because I haven't yet worked out how this gets mapped in the game.
    key_menu_right = KEYD_RIGHTARROW;
    key_menu_left = KEYD_LEFTARROW;
    key_menu_down = KEYD_DOWNARROW;
    key_menu_up = KEYD_UPARROW;
    key_menu_enter = KEYD_ENTER;
    key_menu_escape = KEYD_ESCAPE;

    key_right = KEYD_RIGHTARROW;
    key_left = KEYD_LEFTARROW;
    key_up = KEYD_UPARROW;
    key_down = KEYD_DOWNARROW;
    key_fire = 0x7a;//Z //KEYD_RCTRL;
    key_use = KEYD_SPACEBAR;
    key_speed = 0x78;//X //KEYD_RSHIFT;
    key_strafe = 0x63; //C //KEYD_LALT;

    key_screenshot = 0xffff; //disable

    port_enable(ps2_clock);
    port_enable(ps2_data);
    // set_pad_input_schmitt(ps2_clock); // This broke it. Levels too low
    // set_pad_input_schmitt(ps2_data);

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
