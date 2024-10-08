#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#define BAUD 57600

#define MAX_COMMAND_SIZE 100

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/drivers/led.h"
#include "../include/commands.h"

#define MAIN_LOOP 1

#undef FDEV_SETUP_STREAM
#define FDEV_SETUP_STREAM(p, g, f) \
    {                              \
        .buf = NULL,               \
        .unget = 0,                \
        .flags = f,                \
        .size = 0,                 \
        .len = 0,                  \
        .put = p,                  \
        .get = g,                  \
        .udata = 0                 \
    }


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

void show_prompt(char* command, char reset, command_err_t num) {
    if (reset) {
        printf("\r                                     ");
    }
    printf("\r0x%02x> %s", num, command);
}

int main() {
    FILE uart_output = FDEV_SETUP_STREAM((int (*)(char, FILE*)) uart_putchar, NULL, _FDEV_SETUP_WRITE);
    FILE uart_input = FDEV_SETUP_STREAM(NULL, (int (*)(FILE*)) uart_getchar, _FDEV_SETUP_READ);
    char command[MAX_COMMAND_SIZE] = {0};
    char prev_command[MAX_COMMAND_SIZE] = {0};
    int nCommandCount = 0;
    base_command** command_arr = create_all_commands(&nCommandCount);
    command_err_t prev_err = COMMAND_ERROR_NONE;

    DDRA |= (1 << PA4); // enable onboard LED on porta, direction output

    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;

    while (MAIN_LOOP) {
        show_prompt(command, 0, prev_err);
        int i = 0;
        char input = '\0';

        do {
            input = getchar();
            if (input == 0x0d) break;
            if (input == 0x1b) {
                memset(command, 0, MAX_COMMAND_SIZE);
                memcpy(command, prev_command, MAX_COMMAND_SIZE);
                show_prompt(command, 1, prev_err);

                i = strlen(command);
                getchar();
                getchar();
                continue;
            }

            if (input == 0x7f || input == 0x08) {
                if (i <= 0) {
                    show_prompt(command, 0, prev_err);
                    continue;
                }
                command[--i] = '\0';
                show_prompt(command, 1, prev_err);
                continue;
            }

            if (i >= MAX_COMMAND_SIZE) {
                printf("\r\nbuffer overflow\r\n");
                memset(command, 0, MAX_COMMAND_SIZE);
                i = 0;
                show_prompt(command, 0, prev_err);
                continue;
            }

            command[i++] = input;
            show_prompt(command, 0, prev_err);
        } while(input != 0x0d);
        printf("\r\n");

        int c = 0;
        char commandBuf[MAX_COMMAND_SIZE];
        memset(commandBuf, 0, MAX_COMMAND_SIZE);

        while (command[c] != ':' && command[c] != '\0' && command[c] != 0x0d) {
#if DEBUG
            printf("command[c] is '0x%02x'\r\n", command[c]);
#endif
            c++;
        }
        strncpy(commandBuf, command, c);

        char found_command = 0;
        for (int i = 0; i < nCommandCount; i++) {
            base_command* command_object = (base_command*) command_arr[i];
            if (strcasecmp(commandBuf, command_object->name) == 0) {
                command_err_t error = command_object->execute(command + c);
                found_command = 1;
                prev_err = error;
                break;
            }
        }

        // continue mainloop if a command was found, else fallback to other commands previously implemented
        if (found_command) {
            memcpy(prev_command, command, MAX_COMMAND_SIZE);
            memset(command, 0, MAX_COMMAND_SIZE);

            if (prev_err == COMMAND_ERROR_PANIC) {
                destroy_all_commands(command_arr, TOTAL_COMMAND_COUNT);
                printf("freed commands, exiting...\r\n");
                // TODO: investigate how to put processor into reset?
                break;
            }

            continue;
        } else {
            printf("'%s' is an unknown command\r\n", command);
            prev_err = COMMAND_ERROR_NOT_FOUND;
        }

        memset(command, 0, sizeof(command));
    }
    return 0;
}
