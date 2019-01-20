/*
 * terminal.c
 *
 *  Created on: Jan 19, 2019
 *      Author: nikolay
 */

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "terminal.h"

#define MAX_LINE_LENGTH 79
#define HISTORY_BUFFER_LENGTH 1024
#define WELCOME_LINE "> "

static terminal_send_char send_char;
static wait_char_sent wait_transmition_finished;
static command_received command_callback;
static char receive_buffer[MAX_LINE_LENGTH + 1];
static int receive_buffer_possition = 0;
static char history_buffer[HISTORY_BUFFER_LENGTH] = {0};
static int history_buffer_possition = 0;
static int history_buffer_previous_possition = 0;
static int history_scroll_buffer_possition = 0;
static char history_receive_buffer[sizeof(receive_buffer)] = {0};
static char history_receive_buffer_filled = 0;
static char esc_sequence[4] = {0};
static int esc_sequence_pos = 0;
static bool esc_receive_in_progress = false;
static bool command_in_progress = false;
static bool print_in_progress = false;

static void send_string(const char *s) {
    while(*s) {
        wait_transmition_finished();
        send_char(*s++);
    }
}

static void terminal_send_string(const char *s) {
    if(print_in_progress)
        return;
    send_string(s);
}

static void set_input(const char *line) {
    terminal_send_string("\r\x1B[K");
    terminal_send_string(WELCOME_LINE);
    strncpy(receive_buffer, line, sizeof(receive_buffer));
    receive_buffer_possition = strlen(receive_buffer);
    terminal_send_string(receive_buffer);
}

static void terminal_reset() {
    history_scroll_buffer_possition = history_buffer_possition;
    history_receive_buffer_filled = 0;
    receive_buffer_possition = 0;
    memset(receive_buffer, 0, sizeof(receive_buffer));
}

void terminal_init(terminal_send_char func, wait_char_sent wait_func, command_received callback) {
    send_char = func;
    wait_transmition_finished = wait_func;
    command_callback = callback;
    terminal_reset();
    terminal_send_string(WELCOME_LINE);
}

void terminal_char_receive(char c) {
    int i;
    static char last_recieved = 0;
    if (c == '\n' && last_recieved=='\r') {
        last_recieved = c;
        return;
    }
    last_recieved = c;
    if (c == '\r') {
        c = '\n';
    }
    if (esc_receive_in_progress) {
        if (esc_sequence_pos < sizeof(esc_sequence) - 1) {
            esc_sequence[esc_sequence_pos++] = c;
            esc_sequence[esc_sequence_pos] = 0;
        } else {
            esc_sequence[0] = 0;
        }
        if (esc_sequence_pos > 1 && c >= 0x40 && c <= 0x7e) {
            esc_receive_in_progress = false;
            esc_sequence_pos = 0;
            if (strcmp(esc_sequence, "[A") == 0) { // up
                if (history_scroll_buffer_possition > 1) {
                    if (history_scroll_buffer_possition == history_buffer_possition) {
                        strncpy(history_receive_buffer, receive_buffer, sizeof(history_receive_buffer));
                        history_receive_buffer_filled = 1;
                    }
                    history_scroll_buffer_possition--;
                    while (history_buffer[history_scroll_buffer_possition-1]) {
                        history_scroll_buffer_possition--;
                        if (history_scroll_buffer_possition == 0) {
                            break;
                        }
                    }
                    set_input(&history_buffer[history_scroll_buffer_possition]);
                }
            } else if (strcmp(esc_sequence, "[B") == 0) { // down
                int m = 0;
                while (history_buffer[history_scroll_buffer_possition]) {
                    history_scroll_buffer_possition++;
                    m = 1;
                }
                if (m) {
                    history_scroll_buffer_possition++;
                }
                if (history_buffer[history_scroll_buffer_possition]) {
                    set_input(&history_buffer[history_scroll_buffer_possition]);
                } else if (history_receive_buffer_filled) {
                    set_input(history_receive_buffer);
                    history_receive_buffer_filled = 0;
                }
            } else if (strcmp(esc_sequence, "[C") == 0) { // right
                if (receive_buffer[receive_buffer_possition]) {
                    receive_buffer_possition++;
                    terminal_send_string("\x1B");
                    terminal_send_string(esc_sequence);
                }
            } else if (strcmp(esc_sequence, "[D") == 0) { // left
                if (receive_buffer_possition) {
                    receive_buffer_possition--;
                    terminal_send_string("\x1B");
                    terminal_send_string(esc_sequence);
                }
            } else if (strcmp(esc_sequence, "[3~") == 0) { // delete
                if (receive_buffer[receive_buffer_possition]) {
                    terminal_send_string("\x1B[1P");
                }
                for (i = receive_buffer_possition + 1; i < sizeof(receive_buffer); i++) {
                    if (receive_buffer[i - 1] == 0) {
                        break;
                    }
                    receive_buffer[i - 1] = receive_buffer[i];
                }
            }
        }
        return;
    }

    if (c == '\n') {
        terminal_send_string("\r\n");
        // remove extra spaces
        char *pos = receive_buffer;
        char *from = receive_buffer;
        char last = ' ';
        char quote = 0;
        while (last) {
            if (quote == 0 && last == ' ' && *from == ' ') {
                from++;
            } else {
                last = *from++;
                *pos++ = last;
                if (last =='"'|| last == '\'') {
                    quote = (quote == last) ? 0 : last;
                }
            }
        }
        pos -= 2;
        if (pos > receive_buffer && quote == 0) {
            if (*pos == ' ') {
                *pos = 0;
            }
        }
        if (receive_buffer[0] != 0) {
            if (receive_buffer[0] && strcmp(receive_buffer, &history_buffer[history_buffer_previous_possition]) != 0 ) {
                history_buffer_previous_possition = history_buffer_possition;
                strncpy(&history_buffer[history_buffer_possition], receive_buffer, sizeof(history_buffer) - history_buffer_possition - 2);
                int l = strlen(receive_buffer);
                // put double zero at the end
                history_buffer[history_buffer_possition + l] = 0;
                history_buffer[history_buffer_possition + l + 1] = 0;
                history_buffer_possition += l + 1;
                int n =  MAX_LINE_LENGTH - (sizeof(history_buffer) - history_buffer_possition - 2);
                if (n > 0) {
                    while(history_buffer[n - 1]) {
                        n++;
                    }
                    memmove(history_buffer, &history_buffer[n], sizeof(history_buffer) - n);
                    history_buffer_possition -= n;
                    history_buffer_previous_possition = (history_buffer_previous_possition < n) ? 0 : (history_buffer_previous_possition - n);
                }
            }
            history_scroll_buffer_possition = history_buffer_possition;
            history_receive_buffer_filled = 0;

            command_in_progress = true;
            command_callback(receive_buffer);
            command_in_progress = false;
        }
        receive_buffer_possition = 0;
        memset(receive_buffer, 0, sizeof(receive_buffer));
        terminal_send_string(WELCOME_LINE);
    } else {
        if (c == 0x03) { // ctrl+c
            command_callback("\0x03");
            terminal_send_string("^C\r\n");
            terminal_reset();
            terminal_send_string(WELCOME_LINE);
        } else if (c == 0x1B) { // esp character
            esc_receive_in_progress = true;
        } else if (c == 0x7F || c == 0x08) { // backspace
            if (receive_buffer_possition == 0) {
                return;
            }
            for (i = receive_buffer_possition; i < sizeof(receive_buffer); i++) {
                receive_buffer[i - 1] = receive_buffer[i];
                if (receive_buffer[i] == 0) {
                    break;
                }
            }
            receive_buffer_possition--;
            terminal_send_string("\x1B[D\x1B[1P");
        } else if (c > 0x1F) {
            if (receive_buffer[MAX_LINE_LENGTH - 2] == 0) {
                if (receive_buffer[receive_buffer_possition] != 0) {
                    terminal_send_string("\x1B[@");
                    wait_transmition_finished();
                }
                if (!print_in_progress) {
                    send_char(c);
                }
                char next = c;
                for (i = receive_buffer_possition; i < sizeof(receive_buffer); i++) {
                    char tmp = next;
                    next = receive_buffer[i];
                    receive_buffer[i] = tmp;
                    if (tmp == 0) {
                        break;
                    }
                }
                receive_buffer_possition++;
            }
        }
    }
}

void terminal_printf_line(const char *fmt, ...) {
    va_list    ap;
    char buffer[MAX_LINE_LENGTH * 2];
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    do {
        print_in_progress = true;
        send_string("\r\x1B[K");
        send_string(buffer);
    } while(!print_in_progress); // if interruption was called during print, this flag is false and text is erased, reprint
    send_string("\r\n");
    if(!command_in_progress) {
        send_string(WELCOME_LINE);
        send_string(receive_buffer);
    }
    print_in_progress = false;
    va_end(ap);
}
