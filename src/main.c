#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include <avr/io.h>
#include <util/delay.h>

#include <stdlib.h>
#include <string.h>

int main() {
    // i am the bit master
    // bow before me
    //
    // this will be so much easier than rust

    DDRA |= (1<<PA4);

    char* string = "hello world\n";
    char* new_region = (char*) malloc(strlen(string));

    memcpy(new_region, string, strlen(string));

    // verify that malloc works because it didn't in rust
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == new_region[i]) {
            PORTA |= (1<<PA4);
            _delay_ms(100);

            PORTA &= ~(1<<PA4);
            _delay_ms(100);
        }
    }

    // busy wait
    while(1) {
        PORTA |= (1<<PA4);
        _delay_ms(1000);

        PORTA &= ~(1<<PA4);
        _delay_ms(1000);
    }

    return 0;
}
