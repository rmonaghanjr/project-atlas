#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>

#include "../include/commands.h"
#include "../include/drivers/led.h"
#include "../include/drivers/spidev.h"
#include "../include/drivers/sdcard.h"

#define MAX_COMMAND_SIZE 100

command_err_t onboard_led_command_execute(char* command) {
    char* arg = get_next_argument(command);

    if (strcasecmp(arg, "ON") == 0) {
        set_state(ON);
        printf("done\n");
        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "OFF") == 0) {
        set_state(OFF);
        printf("done\n");
        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "TOGGLE") == 0) {
        set_state(TOGGLE);
        printf("done\n");
        return COMMAND_ERROR_NONE;
    } else {
        int state = get_state();
        if (state == ON) printf("led is on\n");
        else if (state == OFF) printf("led is off\n");
        return COMMAND_ERROR_NONE;
    }
}

command_err_t onboard_spidev_action_execute(char* command) {
    char* arg = get_next_argument(command);

    if (strcasecmp(arg, "WRITE") == 0) { 
        spi_transmit_byte(0xfa);
        printf("sent '0x%02x' to spi device...\r\n", 0xfa);
        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "READ") == 0) {
        printf("recieved '0x%02x' from spi device...\r\n", spi_recieve_byte());
        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "INIT") == 0) {
        spi_init();
        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "TEST") == 0) {
        printf("got back '0x%02x' from spi device...\r\n", spi_test());
        return COMMAND_ERROR_NONE;
    } else {
        return COMMAND_ERROR_BAD_ARG;
    }
}

command_err_t sdcard_command_execute(char* command) {
    char* arg = get_next_argument(command);

    if (strcasecmp(arg, "INIT") == 0) { 
        sdcard_err_t err = sdcard_init();
        if (err != SDCARD_READY) {
            printf("sdcard: init: got error %d", err);
            return COMMAND_ERROR_DEVICE_PROBLEM;
        }

        printf("sdcard: ready to go!\r\n");
        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "CHECK") == 0) {
        R7 res = sdcard_send_if_cond();

        print_r7(res);

        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "READ_OCR") == 0) {
        R7 res = scdard_read_ocr();

        print_r3(res);

        return COMMAND_ERROR_NONE;
    } else if (strcasecmp(arg, "READ_BLOCK") == 0) {
        uint8_t buf[512];
        uint8_t token = 0;
        R1 res1 = sdcard_read_single_block(0x00000000, (uint8_t*) &buf, &token);

        print_r1(res1);

        if (!(token & 0xf0)) {
            print_sdcard_data_token(token);
        } else if (token == 0xff) {
            printf("sdcard: timeout\r\n");
        }

        for (int i = 0; i < 512; i++) {
            if (i % 16 == 0) {
                printf("\r\n");
            }
            printf("%02x ", buf[i]);
        }
        printf("\r\n");

        return COMMAND_ERROR_NONE;
    } else {
        return COMMAND_ERROR_BAD_ARG;
    }
}

char* get_next_argument(char* command) {
    if (command[0] == 0 || command[0] == 13) return NULL;
    if (command[0] == 58) command++;

    int i = 0;
    char* buf = (char*) malloc(MAX_COMMAND_SIZE);
    char seek = 1;

    memset(buf, 0, MAX_COMMAND_SIZE);
    while (seek) {
        if (command[i] == 58 || command[i] == 13 || command[i] == 0) {
            seek = 0;
            break;
        }

        i++;
    }
    strncpy(buf, command, i);
    return buf;
}

base_command** create_all_commands(int* num) {
    base_command** all_commands = (base_command**) malloc(sizeof(base_command*) * TOTAL_COMMAND_COUNT);
    int index = 0;
    
    // initialize command
    base_command* led_command_obj = (base_command*) malloc(sizeof(base_command));
    led_command_obj->name = "LED";
    led_command_obj->execute = &onboard_led_command_execute;

    base_command* spi_command_obj = (base_command*) malloc(sizeof(base_command));
    spi_command_obj->name = "SPI";
    spi_command_obj->execute = &onboard_spidev_action_execute;

    base_command* sdcard_command_obj = (base_command*) malloc(sizeof(base_command));
    sdcard_command_obj->name = "SDCARD";
    sdcard_command_obj->execute = &sdcard_command_execute;


    // add all commands
    all_commands[index++] = (base_command*) led_command_obj;
    all_commands[index++] = (base_command*) spi_command_obj;
    all_commands[index++] = (base_command*) sdcard_command_obj;

    *num = index;
    return all_commands;
}

void destroy_all_commands(base_command** commands, int num) {
    for (int i = 0; i < num; i++) {
        free(commands[i]);
        commands[i] = NULL;
    }

    free(commands);
    commands = NULL;
}