#include <stdint.h>


typedef enum midi_msg_t{
	NOTE_ON,
	NOTE_OFF,
	SYSEX,
	CONTROL_CHANGE,
	PROGRAM_CHANGE,
	PITCH_BEND
} midi_msg_t;

