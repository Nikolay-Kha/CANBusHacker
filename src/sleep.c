/*
 * sleep.c
 *
 *  Created on: Jan 27, 2019
 *      Author: nikolay
 */

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>

#include "sleep.h"

static volatile unsigned int ms_counter = 0;

void SysTick_Handler(void) {
    ms_counter++;
}

void sleep_init() {
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    SysTick_Config(clocks.SYSCLK_Frequency / 1000);
}

void sleep(unsigned int ms) {
    uint32_t ends_at = ms_counter + ms;
    while (ms_counter < ends_at);
}

unsigned int timestamp_ms() {
    return ms_counter;
}

