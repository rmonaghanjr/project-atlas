#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include "./spidev.h"

void sdcard_powerup();
void sdcard_command(uint8_t command, uint32_t arg, uint8_t crc);
uint8_t sdcard_readres1();

uint8_t sdcard_enter_idle_state();