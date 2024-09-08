#include "../../include/drivers/spidev.h"
#include "../../include/drivers/led.h"

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

void spi_init() {
    DDR_SPI |= (1 << PB4) | (1 << MOSI) | (1 << SCK);
    DDRA |= (1 << CS);

    DDR_SPI |= (1 << MISO);

    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

void spi_transmit_byte(uint8_t data) {
    SPDR = data;

    while (!(SPSR & (1 << SPIF)));
}

uint8_t spi_recieve_byte() {
    SPDR = 0xff;

    while(!(SPSR & (1 << SPIF)));

    return SPDR;
}

uint8_t spi_transfer(uint8_t data) {
    SPDR = data;

    set_state(ON);
    while(!(SPSR & (1 << SPIF)));
    printf("spidev: spi_transfer(0x%02x) == 0x%02x\r\n", data, SPDR);
    set_state(OFF);

    return SPDR;
}

uint8_t spi_test() {
    spi_init();

    uint8_t value = 0xfa; // randomly chosen value for demo purposes
    SPI_SLAVE_SELECTED;
    SPDR = value;                  // initiates transfer
    while(!(SPSR & (1 << SPIF))); // wait for SPIF bit to be set
    SPI_SLAVE_DESELECTED;
    return SPDR;
}