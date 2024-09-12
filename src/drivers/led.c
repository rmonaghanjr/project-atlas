#include "../../include/drivers/led.h"

led_state_t led_get_state() {
    switch ((PORTA >> PORTA4) & 1) {
        case 0: return LED_STATE_OFF;
        case 1: return LED_STATE_ON;
        default: return LED_STATE_UNKNOWN;
    }
}

void led_set_state(led_state_t state) {
    led_state_t curr_state = led_get_state();
    
    if (state == curr_state) return;
    if (state == LED_STATE_ON) PORTA |= (1<<PA4);
    if (state == LED_STATE_OFF) PORTA &= ~(1<<PA4);
    if (state == LED_STATE_TOGGLE) {
        if (led_get_state() == LED_STATE_OFF) {
            led_set_state(LED_STATE_ON);
        } else {
            led_set_state(LED_STATE_OFF);
        }
    }
    return;
}