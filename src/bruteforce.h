/*
 * bruteforce.h
 *
 *  Created on: Jan 20, 2019
 *      Author: nikolay
 */

#ifndef BRUTEFORCE_H_
#define BRUTEFORCE_H_

typedef enum {
    BRUTE_OFF,
    BRUTE_FAST,
    BRUTE_BIT,
    BRUTE_BYTE,
    BRUTE_FULL
} BruteType;

void brute_loop();
void bruteforce_run(BruteType type);

#endif /* BRUTEFORCE_H_ */
