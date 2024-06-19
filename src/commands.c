#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/commands.h"
#include "../include/led.h"

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


    // add all commands
    all_commands[index++] = (base_command*) led_command_obj;

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