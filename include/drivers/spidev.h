#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include <avr/io.h>
#include <stdint.h>

// pin definitions
#define DDR_SPI         DDRB
#define PORT_SPI        PORTA
#define CS              PA0
#define MOSI            PB5 
#define MISO            PB6 
#define SCK             PB7

// macros
#define SPI_SLAVE_SELECTED     PORT_SPI  &= ~(1 << CS)
#define SPI_SLAVE_DESELECTED    PORT_SPI |= (1 << CS)

void spi_init();
void spi_transmit_byte(uint8_t data);
uint8_t spi_recieve_byte();
uint8_t spi_transfer(uint8_t data);

uint8_t spi_test();