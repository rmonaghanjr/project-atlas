#ifndef COMMANDS_H
#define COMMANDS_H

#ifndef __AVR_ATmega1284P__
#define __AVR_ATmega1284P__
#endif

#include <string.h>
#include <stdio.h>

#define TOTAL_COMMAND_COUNT 1

typedef enum {
    COMMAND_ERROR_NONE,
    COMMAND_ERROR_UNIMPLEMENTED
} command_err_t;

typedef struct {
    char* name;
    command_err_t (*execute)(char*);
} base_command;

command_err_t onboard_led_command_execute(char* command);

char* get_next_argument(char* command);

base_command** create_all_commands(int* num);
void destroy_all_commands(base_command** commands, int num);

#endif