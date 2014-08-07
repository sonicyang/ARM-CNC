/*
 * timer.c
 *
 *  Created on: 2014/8/7
 *      Author: sonicyang
 */

#include "lpc_types.h"
#include "board.h"
#include "timer.h"

volatile uint32_t sysTickCount = 0;

void SysTick_Init(void){
	uint32_t i = 0;
	for(i = 0; i < MAX_TIMERS; i++)
		timers[i].inUse = FALSE;

	SysTick_Config(SystemCoreClock / 1000); //1ms
}

void SysTick_Handler(void){
	uint32_t i = 0;

	sysTickCount++;	//1ms count

	for(i = 0; i < MAX_TIMERS; i++){
		if(timers[i].inUse && timers[i].targetTime == sysTickCount){
			*(timers[i].flag) = TRUE;
			timers[i].inUse = FALSE;
		}
	}
}

void startTimer(uint32_t time, uint32_t* fla){
	uint32_t i;

	for(i = 0; i < MAX_TIMERS; i++){
		if(!timers[i].inUse){
			timers[i].flag = fla;
			timers[i].inUse = TRUE;
			timers[i].targetTime = sysTickCount + time;
		}
	}
}

void stopTimer(uint32_t* fla){
	uint32_t i;

	for(i = 0; i < MAX_TIMERS; i++){
		if(timers[i].flag == fla){
			timers[i].inUse = FALSE;
		}
	}

	return;
}
