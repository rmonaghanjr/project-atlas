#include "../../include/drivers/sdcard.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#define CMD0        0
#define CMD0_ARG    0x00000000
#define CMD0_CRC    0x94

void sdcard_powerup() {
    SPI_SLAVE_DESELECTED;

    _delay_ms(1);
    for (uint8_t i = 0; i < 10; i++) {
        spi_transfer(0xff);
    }
}

void sdcard_command(uint8_t command, uint32_t arg, uint8_t crc) {
    spi_transfer(command | 0x40);
    
    spi_transfer((uint8_t) (arg >> 24));
    spi_transfer((uint8_t) (arg >> 16));
    spi_transfer((uint8_t) (arg >> 8));
    spi_transfer((uint8_t)(arg));

    spi_transfer(crc | 0x01);
}

uint8_t sdcard_readres1() {
    uint8_t i = 0, res1;

    // keep polling until actual data received
    while((res1 = spi_transfer(0xff)) == 0xff)
    {
        i++;

        // if no data received for 8 bytes, break
        if(i > 8) break;
    }

    return res1;
}

uint8_t sdcard_enter_idle_state() {
    uint8_t res1;

    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);

    sdcard_command(CMD0, CMD0_ARG, CMD0_CRC);
    res1 = sdcard_readres1();   

    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res1;
}