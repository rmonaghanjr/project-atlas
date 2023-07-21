#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#define BAUD 57600

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uart_init() {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_putchar(char c, FILE *stream) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

char uart_getchar(FILE *stream) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}

void show_prompt(char* command, char reset) {
    if (reset) {
        printf("\r                                     ");
    }
    printf("\r@anet3d:~$ %s", command);
}

int main() {
    // i am the bit master
    // bow before me
    //
    // this will be so much easier than rust
    FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
    FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

    DDRA |= (1<<PA4);

    // verify that malloc works because it didn't in rust
    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;

    char command[32];
    char is_shutdown = 0;
    char led_state = 0;

    while (strcmp(command, "shutdown") != 0 && !is_shutdown) {
        show_prompt(command, 0);
        int i = 0;
        char input = '\0';

        do {
            input = getchar();
            if (input == 0xd) break;
            if (input == 0x7f) {
                if (i <= 0) {
                    show_prompt(command, 0);
                    continue;
                }
                command[--i] = '\0';
                show_prompt(command, 1);
                continue;
            }

            if (i >= 32) {
                printf("\r\nbuffer overflow\r\n");
                memset(command, 0, sizeof(command));
                i = 0;
                show_prompt(command, 0);
                continue;
            }

            command[i++] = input;
            show_prompt(command, 0);
        } while(input != 0xd);
        printf("\r\n");

        // command handler
        if (strcmp(command, "led on") == 0) {
            PORTA |= (1<<PA4);
            led_state = 1;
            printf("led enabled\r\n");
        } else if (strcmp(command, "led off") == 0) {
            PORTA &= ~(1<<PA4);
            led_state = 0;
            printf("led disabled\r\n");
        } else if (strcmp(command, "led next") == 0) {
            if (led_state) {
                PORTA &= ~(1<<PA4);
                led_state = 0;
                printf("led disabled\r\n");
            } else {
                PORTA |= (1<<PA4);
                led_state = 1;
                printf("led enabled\r\n");
            }
        } else if (strcmp(command, "led status") == 0) {
            if (led_state) {
                printf("led is enabled\r\n");
            } else {
                printf("led is disabled\r\n");
            }
        } else if (strcmp(command, "shutdown") == 0) {
            printf("shutting down\r\n");
            is_shutdown = 1;
        } else if (strcmp(command, "ping") == 0) {
            printf("pong\r\n");
        } else if (strcmp(command, "help") == 0) {
            printf("available commands:\r\n");
            printf("  led on\r\n");
            printf("  led off\r\n");
            printf("  led next\r\n");
            printf("  led status\r\n");
            printf("  ping\r\n");
            printf("  shutdown\r\n");
        } else {
            printf("'%s' is an unknown command\r\n", command);
        }

        memset(command, 0, sizeof(command));
    }
    return 0;
}
