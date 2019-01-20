/*
 * can.h
 *
 *  Created on: Jan 20, 2019
 *      Author: nikolay
 */

#ifndef CAN_H_
#define CAN_H_

#define CAN_BAUDRATE_KBPS 100

void can_init();
void can_print(const char *text, int id, int dlc, unsigned char *data);
void can_send(const char *line);

#endif /* CAN_H_ */
