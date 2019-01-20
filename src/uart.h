/*
 * uart.h
 *
 *  Created on: Jan 19, 2019
 *      Author: nikolay
 */

#ifndef UART_H_
#define UART_H_

void uart_init(void);
void uart_send(const char b);
void uart_wait_transmition_finished(void);

#endif /* UART_H_ */
