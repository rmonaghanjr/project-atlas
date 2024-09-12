#include "../../include/drivers/sdcard.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#define CMD0        0
#define CMD0_ARG    0x00000000
#define CMD0_CRC    0x94

#define CMD8        8
#define CMD8_ARG    0x0000001AA
#define CMD8_CRC    0x86 //(1000011 << 1)

#define CMD58       58
#define CMD58_ARG   0x00000000
#define CMD58_CRC   0x00

#define CMD55       55
#define CMD55_ARG   0x00000000
#define CMD55_CRC   0x00

#define ACMD41      41
#define ACMD41_ARG  0x40000000
#define ACMD41_CRC  0x00

// read data block
#define CMD17                   17
#define CMD17_CRC               0x00
#define SD_MAX_READ_ATTEMPTS    1563

// write data block
#define CMD24                   24
#define CMD24_ARG               0x00
#define CMD24_CRC               0x00
#define SD_MAX_WRITE_ATTEMPTS   3907

// RESPONSES
// R1
#define PARAM_ERROR(X)      X & 0b01000000
#define ADDR_ERROR(X)       X & 0b00100000
#define ERASE_SEQ_ERROR(X)  X & 0b00010000
#define CRC_ERROR(X)        X & 0b00001000
#define ILLEGAL_CMD(X)      X & 0b00000100
#define ERASE_RESET(X)      X & 0b00000010
#define IN_IDLE(X)          X & 0b00000001

// R7
#define CMD_VER(X)          ((X >> 4) & 0xF0)
#define VOL_ACC(X)          (X & 0x1F)

#define VOLTAGE_ACC_27_33   0b00000001
#define VOLTAGE_ACC_LOW     0b00000010
#define VOLTAGE_ACC_RES1    0b00000100
#define VOLTAGE_ACC_RES2    0b00001000

// R3
#define POWER_UP_STATUS(X)  X & 0x40
#define CCS_VAL(X)          X & 0x40
#define VDD_2728(X)         X & 0b10000000
#define VDD_2829(X)         X & 0b00000001
#define VDD_2930(X)         X & 0b00000010
#define VDD_3031(X)         X & 0b00000100
#define VDD_3132(X)         X & 0b00001000
#define VDD_3233(X)         X & 0b00010000
#define VDD_3334(X)         X & 0b00100000
#define VDD_3435(X)         X & 0b01000000
#define VDD_3536(X)         X & 0b10000000

#define SD_TOKEN_OOR(X)     X & 0b00001000
#define SD_TOKEN_CECC(X)    X & 0b00000100
#define SD_TOKEN_CC(X)      X & 0b00000010
#define SD_TOKEN_ERROR(X)   X & 0b00000001

#define SD_IN_IDLE_STATE    0x01
#define SD_READY            0x00
#define SD_R1_NO_ERROR(X)   X < 0x02

#define SD_START_TOKEN          0xfe
#define SD_ERROR_TOKEN          0x00
#define SD_DATA_ACCEPTED        0x05
#define SD_DATA_REJECTED_CRC    0x0B
#define SD_DATA_REJECTED_WRITE  0x0D

#define SD_BLOCK_LEN            512

sdcard_err_t sdcard_init() {
    R1 res1;
    R7 res7;
    uint8_t attempts = 0;

    spi_init();

    sdcard_powerup();

    while ((res1 = sdcard_enter_idle_state()).value != 0x01) {
        attempts++;

        if (attempts > 10) {
            printf("sdcard: could not power up (%d)\r\n", res1.value);
            return ERROR_SDCARD_POWERUP;
        }
    }

    res7 = sdcard_send_if_cond();
    if (res7.bytes[0] != 0x01) {
        printf("sdcard: failed send_if_cond\r\n");
        return ERROR_SDCARD_IF_COND;
    }
    if (res7.bytes[4] != 0xaa) {
        printf("sdcard: invalid echo pattern 0x%02x (expected 0xaa)\r\n", res7.bytes[4]);
        return ERROR_SDCARD_ECHO;
    }

    attempts = 0;
    do {
        if (attempts > 100) {
            printf("sdcard: could not get into ready state\r\n");
            return ERROR_SDCARD_READY_STATE;
        }

        res1 = sdcard_send_app_cmd();
        if (res1.value < 2) {
            res1 = sdcard_send_op_cond();
        }

        _delay_ms(10);
        attempts++;
    } while (res1.value != 0);

    res7 = scdard_read_ocr();

    if(!(res7.bytes[1] & 0x80)) {
        printf("sdcard: card is not ready after loop\r\n");
        return ERROR_SDCARD_NOT_READY;
    }

    return SDCARD_READY;
}

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

R1 sdcard_readres1() {
    uint8_t i = 0, res1;

    // keep polling until actual data received
    while((res1 = spi_transfer(0xff)) == 0xff)
    {
        i++;

        // if no data received for 8 bytes, break
        if(i > 8) break;
    }

    return (R1) res1;
}

R7 sdcard_readres7() {
    R7 res;
    res.value = 0;

    res.bytes[0] = sdcard_readres1().value;
    if (res.bytes[0] > 1) return res;

    res.bytes[1] = spi_transfer(0xff);
    res.bytes[2] = spi_transfer(0xff);
    res.bytes[3] = spi_transfer(0xff);
    res.bytes[4] = spi_transfer(0xff);

    return res;
}

void print_r1(R1 result) {
    uint8_t res = result.value;

    // should never be 1 according to https://users.ece.utexas.edu/~valvano/EE345M/SD_Physical_Layer_Spec.pdf
    if(res & 128) {
        printf("error: msb = 1\r\n");
        return; 
    }

    if(res == 0) { 
        printf("sdcard: card ready\r\n");
        return;
    }

    if(PARAM_ERROR(res))
        printf("scdard: parameter error\r\n");
    if(ADDR_ERROR(res))
        printf("sdcard: address error\r\n");
    if(ERASE_SEQ_ERROR(res))
        printf("sdcard: erase sequence error\r\n");
    if(CRC_ERROR(res))
        printf("sdcard: crc error\r\n");
    if(ILLEGAL_CMD(res))
        printf("sdcard: illegal command\r\n");
    if(ERASE_RESET(res))
        printf("sdcard: erase reset error\r\n");
    if(IN_IDLE(res))
        printf("sdcard: in idle state\r\n");
}

void print_r3(R7 result) {
    uint8_t* res = result.bytes;

    print_r1((R1) res[0]);

    if(res[0] > 1) return;

    printf("sdcard: card power up status: ");
    if(POWER_UP_STATUS(res[1]))
    {
        printf("READY\r\n");
        printf("sdcard: ccs status: %d\r\n", CCS_VAL(res[1]) ? 1 : 0);
    }
    else
    {
        printf("BUSY\r\n");
    }

    printf("sdcard: vdd window: ");
    if(VDD_2728(res[3])) printf("2.7-2.8, ");
    if(VDD_2829(res[2])) printf("2.8-2.9, ");
    if(VDD_2930(res[2])) printf("2.9-3.0, ");
    if(VDD_3031(res[2])) printf("3.0-3.1, ");
    if(VDD_3132(res[2])) printf("3.1-3.2, ");
    if(VDD_3233(res[2])) printf("3.2-3.3, ");
    if(VDD_3334(res[2])) printf("3.3-3.4, ");
    if(VDD_3435(res[2])) printf("3.4-3.5, ");
    if(VDD_3536(res[2])) printf("3.5-3.6");
    printf("\r\n");
}

void print_r7(R7 result) {
    uint8_t* res = result.bytes;

    print_r1((R1) res[0]);
    if(res[0] > 1) return;

    printf("sdcard: command version: 0x%08x\r\n", CMD_VER(res[1]));

    printf("sdcard: voltage accepted: ");
    if(VOL_ACC(res[3]) == VOLTAGE_ACC_27_33)
        printf("2.7-3.6V\r\n");
    else if(VOL_ACC(res[3]) == VOLTAGE_ACC_LOW)
        printf("LOW VOLTAGE\r\n");
    else if(VOL_ACC(res[3]) == VOLTAGE_ACC_RES1)
        printf("RESERVED\r\n");
    else if(VOL_ACC(res[3]) == VOLTAGE_ACC_RES2)
        printf("RESERVED\r\n");
    else
        printf("NOT DEFINED\r\n");

    printf("sdcard: echo: 0x%08x\r\n", res[4]);
}

void print_sdcard_data_token(uint8_t token) {
    if (SD_TOKEN_OOR(token)) {
        printf("sdcard: data out of range\r\n");
    }

    if (SD_TOKEN_CECC(token)) {
        printf("sdcard: card ecc failed\r\n");
    }

    if (SD_TOKEN_CC(token)) {
        printf("sdcard: cc error\r\n");
    }

    if (SD_TOKEN_ERROR(token)) {
        printf("sdcard: error\r\n");
    }
}

R1 sdcard_enter_idle_state() {
    R1 res1;

    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);

    sdcard_command(CMD0, CMD0_ARG, CMD0_CRC);
    res1.value = sdcard_readres1().value;   

    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res1;
}

R7 sdcard_send_if_cond() {
    R7 res;
    
    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);

    sdcard_command(CMD8, CMD8_ARG, CMD8_CRC);

    res = sdcard_readres7();

    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res;
}

R7 scdard_read_ocr() {
    R7 res;
    // assert chip select
    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);

    // send CMD58
    sdcard_command(CMD58, CMD58_ARG, CMD58_CRC);

    // read response
    res = sdcard_readres7();

    // deassert chip select
    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res;
}

R1 sdcard_send_app_cmd() {
    R1 res;

    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);
    
    sdcard_command(CMD55, CMD55_ARG, CMD55_CRC);

    // read response
    res = sdcard_readres1();

    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res;
}

R1 sdcard_send_op_cond() {
    R1 res;

    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);
    
    sdcard_command(ACMD41, ACMD41_ARG, ACMD41_CRC);

    // read response
    res = sdcard_readres1();

    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res;
}

/*******************************************************************************
 Read single 512 byte block
 token = 0xFE - Successful read
 token = 0x0X - Data error
 token = 0xFF - Timeout
*******************************************************************************/
R1 sdcard_read_single_block(uint32_t addr, uint8_t *buf, uint8_t *token) {
    R1 res1;
    uint8_t read;
    uint16_t read_attempts;

    // set token to none
    *token = 0xff;

    spi_transfer(0xff);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xff);

    // send CMD17
    sdcard_command(CMD17, addr, CMD17_CRC);

    // read R1
    res1 = sdcard_readres1();

    // get response from card
    if (res1.value != 0xff) {
        // wait for a response token (timeout = 100ms)
        read_attempts = 0;
        while (++read_attempts != SD_MAX_READ_ATTEMPTS) {
            if ((read = spi_transfer(0xff)) != 0xff) break;
        } 

        // if response token is 0xFE
        if (read == 0xfe) {
            // 512 byte block
            for(uint16_t i = 0; i < 512; i++) {
                *buf++ = spi_transfer(0xff);
            }

            // read 16-bit CRC
            spi_transfer(0xff);
            spi_transfer(0xff);
        }

        // set token to card response
        *token = read;
    }

    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xFF);

    return res1;
}

/*******************************************************************************
 Write single 512 byte block
 token = 0x00 - busy timeout
 token = 0x05 - data accepted
 token = 0xFF - response timeout
*******************************************************************************/
R1 sdcard_write_single_block(uint32_t addr, uint8_t *buf, uint8_t *token) {
    uint8_t attempts, read;
    R1 res;

    // set token to none
    *token = 0xFF;

    // assert chip select
    spi_transfer(0xFF);
    SPI_SLAVE_SELECTED;
    spi_transfer(0xFF);

    // send CMD24
    sdcard_command(CMD24, addr, CMD24_CRC);

    // read response
    res = sdcard_readres1();

    // if no error
    if (res.bytes[0] == SD_READY) {
        // send start token
        spi_transfer(SD_START_TOKEN);

        // write buffer to card
        for (uint16_t i = 0; i < SD_BLOCK_LEN; i++) {
            spi_transfer(buf[i]);
        }

        // wait for a response (timeout = 250ms)
        attempts = 0;
        while (++attempts != SD_MAX_WRITE_ATTEMPTS) {
            if ((read = spi_transfer(0xff)) != 0xff) {
                *token = 0xFF; 
                break; 
            }
        }

        // if data accepted
        if ((read & 0x1f) == 0x05) {
            // set token to data accepted
            *token = 0x05;

            // wait for write to finish (timeout = 250ms)
            attempts = 0;
            while (spi_transfer(0xff) == 0x00) {
                if (++attempts == SD_MAX_WRITE_ATTEMPTS) {
                    *token = 0x00;
                    break;
                }
            }
        }
    }

    // deassert chip select
    spi_transfer(0xff);
    SPI_SLAVE_DESELECTED;
    spi_transfer(0xff);

    return res;
}