/*
 * terminal.h
 *
 *  Created on: Jan 19, 2019
 *      Author: nikolay
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdarg.h>

// Simple terminal implementation. Can navigate in command prompt throw line and commands history.

typedef void (*terminal_send_char)(const char);
typedef void (*wait_char_sent)(void);
typedef void (*command_received)(const char *);

void terminal_init(terminal_send_char func, wait_char_sent wait_func, command_received callback);
void terminal_char_receive(char c);
void terminal_printf_line(const char *fmt, ...);

#endif /* TERMINAL_H_ */
