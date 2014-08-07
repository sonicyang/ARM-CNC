/*
 * timer.h
 *
 *  Created on: 2014/8/7
 *      Author: sonicyang
 */

#ifndef TIMER_H_
#define TIMER_H_

#define MAX_TIMERS 16

extern volatile uint32_t sysTickCount;

void SysTick_Init(void);

typedef struct {
	uint8_t		inUse;
	uint32_t	targetTime;
	uint32_t*	flag;
} TIMER_T;

TIMER_T	timers[MAX_TIMERS];

void startTimer(uint32_t time, uint32_t* fla);

void stopTimer(uint32_t* flas);

#endif /* TIMER_H_ */
