/*
 * main.c
 *
 *  Created on: Jan 19, 2019
 *      Author: nikolay
 */

#include <string.h>

#include <stm32f10x.h>

#include "terminal.h"
#include "bruteforce.h"
#include "can.h"
#include "uart.h"

static void do_command(const char *line) {
    if (strcmp(line, "brute_fast") == 0) {
        bruteforce_run(BRUTE_FAST);
    } else if (strcmp(line, "brute_bit") == 0) {
        bruteforce_run(BRUTE_BIT);
    } else if (strcmp(line, "brute_byte") == 0) {
        bruteforce_run(BRUTE_BYTE);
    } else if (strcmp(line, "brute_full") == 0) {
        bruteforce_run(BRUTE_FULL);
    } else if (strcmp(line, "\0x0C") == 0) {
        bruteforce_run(BRUTE_OFF);
    } else {
        can_send(line);
    }
}

int main(void) {
    uart_init();
    terminal_init(uart_send, uart_wait_transmition_finished, do_command);
    terminal_printf_line("\r\n***** Welcome to CANBusHacker *****");
    terminal_printf_line("CAN baudrate is %d Kbps", CAN_BAUDRATE_KBPS);
    can_init();
    while(1) {
        brute_loop();
    }
}
