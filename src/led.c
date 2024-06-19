#include "../include/led.h"

led_state_t get_state() {
    switch((PORTA >> PORTA4) & 1) {
        case 0: return OFF;
        case 1: return ON;
        default: return UNKNOWN;
    }
}

void set_state(led_state_t state) {
    led_state_t curr_state = get_state();
    
    if (state == curr_state) return;
    if (state == ON) PORTA |= (1<<PA4);
    if (state == OFF) PORTA &= ~(1<<PA4);
    if (state == TOGGLE) {
        if (get_state() == OFF) {
            set_state(ON);
        } else {
            set_state(OFF);
        }
    }
    return;
}