/*
 * message.c
 *
 *  Created on: Jan 20, 2019
 *      Author: nikolay
 */

#include "can.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <stm32f10x_can.h>

#include "terminal.h"

void can_init(void) {
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CAN_DeInit(CAN1);
    CAN_InitTypeDef CAN_InitStructure;
    CAN_StructInit(&CAN_InitStructure);
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = DISABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = ENABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//TODO TODO TODO // CAN_Mode_Normal;

    // PCLK1 and APB1 clock is 36MHz
    #if CAN_BAUDRATE_KBPS == 1000
        CAN_InitStructure.CAN_Prescaler = 3; // 36000000 / 3 / 1000000 = 12 quantas
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq; // sample point (1 + 8) / 12 = 0.75 = 75%
        CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    #elif CAN_BAUDRATE_KBPS == 500
        CAN_InitStructure.CAN_Prescaler = 6; // 36000000 / 6 / 500000 = 12 quantas
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq; // sample point (1 + 8) / 12 = 0.75 = 75%
        CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    #elif CAN_BAUDRATE_KBPS == 250
        CAN_InitStructure.CAN_Prescaler = 9; // 36000000 / 9 / 250000 = 16 quantas
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq; // sample point (1 + 11) / 16 = 0.75 = 75%
        CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
    #elif CAN_BAUDRATE_KBPS == 125
        CAN_InitStructure.CAN_Prescaler = 18; // 36000000 / 18 / 125000 = 16 quantas
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_11tq; // sample point (1 + 11) / 16 = 0.75 = 75%
        CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
    #elif  CAN_BAUDRATE_KBPS == 100
        CAN_InitStructure.CAN_Prescaler = 30; // 36000000 / 30 / 100000 = 12 quantas
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq; // sample point (1 + 8) / 12 = 0.75 = 75%
        CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    #elif CAN_BAUDRATE_KBPS == 50
        CAN_InitStructure.CAN_Prescaler = 60; // 36000000 / 60 / 50000 = 12 quantas
        CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
        CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq; // sample point (1 + 8) / 12 = 0.75 = 75%
        CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    #else
        #error "No baund rate specified."
    #endif
    CAN_Init(CAN1, &CAN_InitStructure);

    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_FOV0, ENABLE);
}

void can_print(const char *text, int id, int dlc, unsigned char *data) {
    if (dlc > 8) {
        terminal_printf_line("Message data is more than 8 bytes, CAN is not allowed that");
        return;
    }
    char buf[8 * 2 + 1] = {0};
    for (int i = 0; i < dlc; i++) {
        sprintf(&buf[i * 2], "%02X", data[i]);
    }
    terminal_printf_line("%s: %X#%s [%d %s]", text, id, buf, dlc,
            (dlc == 1) ? "byte" : "bytes");
}

void can_send(const char *line) {
    CanTxMsg message = { 0 };
    char buf[18];
    const char *line_check = line;
    int pound_found = 0;
    int error_found = 0;
    unsigned int id;
    while(*line_check) {
        if (!isxdigit(*line_check)) {
            if (!pound_found && *line_check == '#') {
                pound_found = 1;
            } else {
                error_found = 1;
                break;
            }
        }
        line_check++;
    }
    if (error_found || !pound_found || sscanf(line, "%x#%17s", &id, buf) == EOF) {
        terminal_printf_line("Wrong CAN message, use format {id}#{data}, example 12A#11FF22DD");
        return;
    }
    int buflen = strlen(buf);
    if (buflen > 16 || (buflen % 2) != 0) {
        terminal_printf_line("Wrong CAN message data size, can be 0-8 bytes, use hex string");
        return;
    }
    if (id > 0x7FF) {
        terminal_printf_line("Wrong CAN message id, can be between 0 to 7FF");
        return;
    }

    message.ExtId = 0x0;
    message.RTR = CAN_RTR_Data;
    message.IDE = CAN_Id_Standard;
    message.StdId = id;
    message.DLC = buflen / 2;
    for (int i = 0; i < buflen / 2; i++) {
        sscanf(&buf[i * 2], "%2hhx", &message.Data[i]);
    }
    CAN_Transmit(CAN1, &message);
    can_print("CAN send", message.StdId, message.DLC, message.Data);
}

void USB_LP_CAN_RX0_IRQHandler(void) {
    CanRxMsg message = { 0 };
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET) {
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
        CAN_Receive(CAN1, CAN_FIFO0, &message);
        if (message.IDE == CAN_Id_Standard) {
            // CAN data received
            can_print("CAN received", message.StdId, message.DLC, message.Data);
        }
    }

    // clear and report error if any
    if (CAN_GetITStatus(CAN1, CAN_IT_FOV0) == SET) {
        CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0);
        terminal_printf_line("CAN buffer overflow");
        CAN_ClearFlag(CAN1, CAN_FLAG_FOV0);
    }
}
