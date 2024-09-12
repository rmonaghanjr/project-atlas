#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include <avr/io.h>

typedef enum {
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_TOGGLE,
    LED_STATE_UNKNOWN
} led_state_t;

led_state_t led_get_state();
void led_set_state(led_state_t state);