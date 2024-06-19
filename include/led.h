#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include <avr/io.h>

typedef enum {
    OFF,
    ON,
    TOGGLE,
    UNKNOWN
} led_state_t;

led_state_t get_state();
void set_state(led_state_t state);