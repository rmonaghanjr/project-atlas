#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include "./spidev.h"

typedef union {
    uint8_t value;
    uint8_t bytes[1];
} R1;

typedef union {
    uint64_t value;
    uint8_t bytes[8];
} R7; 

typedef enum {
    SDCARD_READY,
    ERROR_SDCARD_POWERUP,
    ERROR_SDCARD_IF_COND,
    ERROR_SDCARD_ECHO,
    ERROR_SDCARD_READY_STATE,
    ERROR_SDCARD_NOT_READY,
} sdcard_err_t;

sdcard_err_t sdcard_init();

void sdcard_powerup();
void sdcard_command(uint8_t command, uint32_t arg, uint8_t crc);

R1 sdcard_readres1();
R7 sdcard_readres7();

void print_r1(R1 result);
void print_r3(R7 result);
void print_r7(R7 result);

void print_sdcard_data_token(uint8_t token);

R1 sdcard_enter_idle_state();
R7 sdcard_send_if_cond();
R7 scdard_read_ocr();
R1 sdcard_send_app_cmd();
R1 sdcard_send_op_cond();
R1 sdcard_read_single_block(uint32_t addr, uint8_t *buf, uint8_t *token);
R1 sdcard_write_single_block(uint32_t addr, uint8_t *buf, uint8_t *token);