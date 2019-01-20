/*
 * bruteforce.c
 *
 *  Created on: Jan 20, 2019
 *      Author: nikolay
 */

#include <stdbool.h>
#include <stdio.h>

#include "bruteforce.h"

#include "can.h"

static BruteType brute_in_progress = BRUTE_OFF;

static int base_brute(const char *fmt,  unsigned long long data) {
    char buf[32];
    for (int i = 0x0; i <= 0x7FF; i++) {
        if (brute_in_progress == BRUTE_OFF) {
            return false;
        }
        sprintf(buf, fmt, i, data);
        can_send(buf);
        //for(int h = 0; h< 0xFFFFFF; h++);
    }
    return true;
}

static void brute_byte(void) {
    char fmt[32] = "%X#";
    for (char c = 'F'; c > '0'; c--){
        for (int bytes = 1; bytes <= 8; bytes ++) {
            for (int j = 0; j < (1 << (bytes * 2)) ; j++) {
                char *fmt_ptr = &fmt[3];
                for (int i = 0; i < bytes * 2; i++) {
                    *fmt_ptr++ = (j & (1 << i)) ? c : '0';
                }
                *fmt_ptr = 0;
                if (!base_brute(fmt, 0)) {
                    return;
                }
            }
        }
    }
}

static void brute_full(void) {
    char fmt[32];
    for (int bytes = 1; bytes <= 6; bytes ++) { // just 6 byte brute-force
        // do not increase more than 8 bytes, long long will be overloaded
        sprintf(fmt, "%%X#%%0%dllX", bytes * 2);
        for (unsigned long long j = 0; j < ((unsigned long long)0x1 << (bytes * 8)); j++) {
            if (!base_brute(fmt, j)) {
                return;
            }
        }
    }
}

static void brute_bit(void) {
    char fmt[32];
    for (int bytes = 1; bytes <= 8; bytes ++) {
        sprintf(fmt, "%%X#%%0%dllX", bytes * 2);
        for (int j = 0; j < bytes * 8; j++) {
            if (!base_brute(fmt, (unsigned long long)1 << j)) {
                return;
            }
        }
    }
}

static void brute_fast(void) {
    char fmt[32];
    for (int bytes = 1; bytes <= 8; bytes ++) {
        sprintf(fmt, "%%X#%%0%dllX", bytes * 2);
        for (int j = 0; j < bytes * 2; j++) {
            if (!base_brute(fmt, (unsigned long long)0xF << (j * 4))) {
                return;
            }
        }
    }
}

void brute_loop() {
    switch (brute_in_progress) {
    case BRUTE_FAST:
        brute_fast();
        brute_in_progress = BRUTE_OFF;
        break;
    case BRUTE_BIT:
        brute_bit();
        brute_in_progress = BRUTE_OFF;
        break;
    case BRUTE_BYTE:
        brute_byte();
        brute_in_progress = BRUTE_OFF;
        break;
    case BRUTE_FULL:
        brute_full();
        brute_in_progress = BRUTE_OFF;
        break;
    case BRUTE_OFF:
        break;
    }
}

void bruteforce_run(BruteType type) {
    brute_in_progress = type;
}
