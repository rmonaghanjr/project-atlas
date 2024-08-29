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

#define MOSI PB5
#define SCK PB7
#define SS PB4
#define MISO PB6

#define TEST(n,b) (((n)&_BV(b))!=0)

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

void spi_init() {
    DDRB |= (1<<MOSI)|(1<<SCK)|(1<<SS);	/* Make MOSI, SCK, SS 
						as Output pin */
	DDRB &= ~(1<<MISO);			/* Make MISO pin 
						as input pin */
	PORTB |= (1<<SS);			/* Make high on SS pin */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);	/* Enable SPI in master mode
						with Fosc/16 */
	SPSR &= ~(1<<SPI2X);	
}

char spi_rec() {
    SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));	/* Wait till reception complete */
	return (SPDR);
}

void spi_send(char data) {
	SPDR = data;			/* Write data to SPI data register */
	while(!(SPSR & (1<<SPIF)));	/* Wait till transmission complete */
  }

void show_prompt(char* command, char reset) {
    if (reset) {
        printf("\r                                     ");
    }
    printf("\r> %s", command);
}

int main() {
    FILE uart_output = FDEV_SETUP_STREAM((int (*)(char, FILE*)) uart_putchar, NULL, _FDEV_SETUP_WRITE);
    FILE uart_input = FDEV_SETUP_STREAM(NULL, (int (*)(FILE*)) uart_getchar, _FDEV_SETUP_READ);

    DDRA |= (1 << PA4); // enable onboard LED on porta, direction output

    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;

    // spi_init();

    char command[MAX_COMMAND_SIZE];
    int nCommandCount = 0;
    base_command** command_arr = create_all_commands(&nCommandCount);

    while (MAIN_LOOP) {
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

            if (i >= MAX_COMMAND_SIZE) {
                printf("\r\nbuffer overflow\r\n");
                memset(command, 0, MAX_COMMAND_SIZE);
                i = 0;
                show_prompt(command, 0);
                continue;
            }

            command[i++] = input;
            show_prompt(command, 0);
        } while(input != 0xd);
        printf("\r\n");

        int c = 0;
        char commandBuf[MAX_COMMAND_SIZE];
        memset(commandBuf, 0, MAX_COMMAND_SIZE);

        while (command[c] != ':' || command[c] == '\0' || command[c] == 0x0d) c++;
        strncpy(commandBuf, command, c);

        char found_command = 0;
        for (int i = 0; i < nCommandCount; i++) {
            base_command* command_object = (base_command*) command_arr[i];
            if (strcasecmp(commandBuf, command_object->name) == 0) {
                command_err_t error = command_object->execute(command + c);
                found_command = 1;
                break;
            }
        }

        // continue mainloop if a command was found, else fallback to other commands previously implemented
        if (found_command) continue;

        // command handler
        if (strcmp(command, "shutdown") == 0) {
            printf("shutting down\r\n");
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
        } else if (strcmp(command, "spi init") == 0) {
            spi_init();
        } else if (strcmp(command, "spi write") == 0) {
            spi_send('r');
        } else if (strcmp(command, "spi read") == 0) {
            printf("0x%x\n", spi_rec());
        } else {
            printf("'%s' is an unknown command\r\n", command);
        }

        memset(command, 0, sizeof(command));
    }
    return 0;
}
