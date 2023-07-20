#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include <avr/io.h>
#include <util/delay.h>

int main() {
    // i am the bit master
    // bow before me
    //
    // this will be so much easier than rust

    DDRA |= (1<<PA4);
    while(1) {
        PORTA |= (1<<PA4);
        _delay_ms(1000);

        PORTA &= ~(1<<PA4);
        _delay_ms(1000);
    }

    return 0;
}
