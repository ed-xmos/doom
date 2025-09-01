#include "ps2.h"
#include <xcore/port.h>
#include <xcore/select.h>

static char ps2lookupUSB[0x85] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2b, 0x35, 0x00,
    0x00, 0xe0, 0x02, 0x00, 0x39, 0x14, 0x1e, 0x00,
    0x00, 0x00, 0x1d, 0x16, 0x04, 0x1a, 0x1f, 0x00,
    0x00, 0x06, 0x1b, 0x07, 0x08, 0x21, 0x20, 0x00,
    0x00, 0x2c, 0x19, 0x09, 0x17, 0x15, 0x22, 0x00,
    0x00, 0x11, 0x05, 0x0b, 0x0a, 0x1c, 0x23, 0x00,
    0x00, 0x00, 0x10, 0x0d, 0x18, 0x24, 0x25, 0x00,
    0x00, 0x36, 0x0E, 0x0C, 0x12, 0x27, 0x26, 0x00,
    0x00, 0x37, 0x38, 0x0F, 0x33, 0x13, 0x2D, 0x00,
    0x00, 0x00, 0x34, 0x00, 0x2F, 0x2E, 0x00, 0x00,
    0xE4, 0xE5, 0x28, 0x30, 0x00, 0x31, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00,
    0x00, 0x59, 0x00, 0x5C, 0x5F, 0x00, 0x00, 0x00,
    0x62, 0x63, 0x5A, 0x5D, 0x5E, 0x60, 0x53, 0x54,
    0x00, 0x58, 0x5B, 0x85, 0x57, 0x61, 0x55, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x56,
};

static char ps2lookupASCII[0x85] = {    
		0, //                        
		0, // F9                     
		0, //                        
		0, // F5    
		0, // F3                     
		0, // F1                     
		0, // F2                     
		0, // F12                    
		0, //                        
		0, // F10                    
		0, // F8                     
		0, // F6                     
		0, // F4                     
		0x09, // TAB                    
		0x60, // ` or |                        
		0, //                        
		0, //                        
		0, // Left ALT               
		0, //PS2_SHIFT, // Left SHIFT               
		0, //                        
		0, //PS2_CTRL, // Left Ctrl                 
		'q', // Q                              
		'1', // 1 or !                         
		0, //                        
		0, //                        
		0, //                        
		'z', // Z                              
		's', // S                              
		'a', // A                              
		'w', // W                              
		'2', // 2 or @                         
		0, //                        
		0, //                        
		'c', // C                              
		'x', // X                              
		'd', // D                              
		'e', // E                              
		'4', // 4 or $                         
		'3', // 3 or ¬£                         
		0, //                        
		0, //                        
		' ', // Space                          
		'v', // V                              
		'f', // F                              
		't', // T                              
		'r', // R                              
		'5', // 5 or %                         
		0, //                        
		0, //                        
		'n', // N                              
		'b', // B                              
		'h', // H                              
		'g', // G                              
		'y', // Y                              
		'6', // 6 or ^                         
		0, //                        
		0, //                        
		0, //                        
		'm', // M                              
		'j', // J                              
		'u', // U                              
		'7', // 7 or &                         
		'8', // 8 or *                         
		0, //                        
		0, //                        
		',', // , or <                         
		'k', // K                              
		'i', // I                              
		'o', // o                        
		'0', // 0 or ) 
		'9', // 9 or (                                    
		0, //                                      
		0, //                         
		'.', // . or >
		'/', // / or ?
		'l', // L                                              
		';', // ; or :                              
		'p', // p                         
		'-', // - or _                     
		0, //                        
		0, //                        
		0, // 
		0x27,        //  ' or @                                          
		0, //                        
		'[', // [ or {                         
		'=', // = OR +                         
		0, //                        
		0, // Caps Lock                      
		0, //PS2_CAPS, 
		0, //PS2_SHIFT, // Right Shift 
		0x0D, // Enter                     
		']', // ] or }                                     
		0, // 
		'#', // # or |                                 
		0, //                        
		0, //                        
		0, //
		'\\',        // \ or | UK KEYBOARD                                    
		0, //                        
		0, //                        
		0, //                        
		0, //
		0x08, // Backspace                                  
		0, //                        
		0, // NUM - 1 or END         
		'1', //                        
		0, // NUM - 4 or LEFT        
		'4', // NUM - 7 or HOME        
		'7', //                        
		0, //                        
		0, //                         
		'.', // NUM - . or DEL
		'0', // NUM - 0 or INS         
		'.', // NUM - 2 or DOWN        
		'2', // NUM - 5                
		'5', // NUM - 6 or RIGHT       
		'6', // NUM - 8 or UP          
		'8', // F11                       
		0x1B, // ESC    76                   
		0, //PS2_NUM, // NUM LOCK                    
		0, // NUM - + (Plus)         
		'+', // NUM 3 or PAGE DOWN     
		'3', // NUM - - (Minus)        
		'-', // NUM - *                
		'*', // NUM - 9 or PAGE UP     
		'9', // SCROLL LOCK            
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0, // F7                     
		0, //                        
		0, //                        
};
/*
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0, //                        
		0
};
*/


void ps2HandlerInit(port_t ps2_clock, struct ps2state *state) {
    state->overrunErrors = 0;
    state->parityErrors = 0;
    state->stopErrors = 0;
    state->valid = 0;
    state->bits = 0;
    state->mode = START_BIT;
    state->clockValue = port_in(ps2_clock);
    state->clockValue = 1;
    state->modifier = 0;
    state->released = 0;
}


int ps2USB(unsigned int value) {
    return value < sizeof(ps2lookupUSB) ? ps2lookupUSB[value] : -1;
}

int ps2ASCII(unsigned int modifier, unsigned int value) {
    value = value < sizeof(ps2lookupASCII) ? ps2lookupASCII[value] : -1;
    if (modifier & PS2_MODIFIER_SHIFT) {
        if (value >= 'a' && value <= 'z') {
            return value - 0x20;
        }
        if (value >= '1' && value <= '0') {
            return value - 0x10;
        }
        return value;
    } else if (modifier & PS2_MODIFIER_SHIFT) {
        if (value >= 'a' && value <= 'z') {
            return value - 0x60;
        }
    }
    return value;
}

void ps2Interpret(	struct ps2state *state,
					unsigned *action,
					unsigned *modifier,
					unsigned *key) {

    unsigned result;
    if (!state->valid) {
    	*action = PS2_NONE;
    	*modifier = state->modifier;
    	*key = 0;
        return;
    }
    state->valid = 0;
    *key = state->value;
    if (*key == INT_PS2_RELEASE) {
        state->released = 1;
        result = PS2_NONE;
    } else if (*key == INT_PS2_EXT) {
        result = PS2_NONE;
    } else {
        switch (*key) {
        case INT_PS2_SHIFT:
            if (state->released) {
                state->modifier &= ~PS2_MODIFIER_SHIFT;
            } else {
                state->modifier |= PS2_MODIFIER_SHIFT;
            }
            result = PS2_NONE;
            break;
        case INT_PS2_CTRL:
            if (state->released) {
                state->modifier &= ~PS2_MODIFIER_CTRL;
            } else {
                state->modifier |= PS2_MODIFIER_CTRL;
            }
            result = PS2_NONE;
            break;
        case INT_PS2_ALT:
            if (state->released) {
                state->modifier &= ~PS2_MODIFIER_ALT;
            } else {
                state->modifier |= PS2_MODIFIER_ALT;
            }
            result = PS2_NONE;
            break;
        default:
            if (state->released) {
                result = PS2_RELEASE;
            } else {
                result = PS2_PRESS;
            }
            break;
        }
        state->released = 0;
    }

    *action = result;
    *modifier = state->modifier;

    return;
}


void handle_clock_helper(port_t ps2_clock, port_t ps2_data, int clockBit, struct ps2state *state){
	// Grab the clock value
	int new = port_in(ps2_clock);
	if ((~state->clockValue & new) >> clockBit & 1) { // seen rising edge
	        state->bit = port_in(ps2_data);
	        switch(state->mode) {
	        case START_BIT: 
	            if (state->bit == 0) {
	                state->mode = BIT0;
	            }
	            break;
	        case BIT0:
	        case BIT1:
	        case BIT2:
	        case BIT3:
	        case BIT4:
	        case BIT5:
	        case BIT6:
	        case BIT7:
	            state->bits >>= 1;
	            if (state->bit) state->bits |= 0x80;
	            state->mode++;
	            break;
	        case PARITY_BIT:
	        {
	            unsigned int parity;
	            parity = state->bits | state->bit<<8;

	            unsigned parity_setting = 0x1;
	            unsigned poly = 0;
	            // crc32(parity, 0x1, 0);
	            asm volatile("crc32 %0, %2, %3" : "=r" (parity) : "0" (parity), "r" (parity_setting), "r" (poly));
	        
	            if (parity == 1) {
	                state->mode = STOP_BIT;
	            } else {
	                state->parityErrors++;
	                state->mode = START_BIT;
	            }
	        }
	            break;
	        case STOP_BIT: 
	            if (state->bit == 1) {
	                if (state->valid) {
	                    state->overrunErrors++;
	                }
	                state->value = state->bits;
	                state->valid = 1;
	            } else {
	                state->stopErrors++;
	            }
	            state->mode = START_BIT;
	            break;
	        }
	    } else {
	        // falling clock or some other change to port, irrelevant to PS/2 clock.
	    }
	    state->clockValue = new;
}

void ps2Handler(port_t ps2_clock, port_t ps2_data, int clockBit, struct ps2state *state) {

	port_set_trigger_in_not_equal(ps2_clock, state->clockValue);
	SELECT_RES(
	    CASE_THEN(ps2_clock, handle_clock)
    )
    {
        handle_clock:
        {
        	handle_clock_helper(ps2_clock, ps2_data, clockBit, state);
        }
        break;

    }
}
