/*
 * motorController.c
 *
 *  Created on: 2014/8/5
 *      Author: sonicyang
 */

#include "board.h"
#include "motorController.h"
#include "ring_buffer.h"

void motorControllerInit(void){
	uint32_t timerFreq;

	RingBuffer_Init(&movebuf, movebuf_base, sizeof(MOVE_T), MOVE_RB_SIZE);
	RingBuffer_Init(&vectorbuf, vectorbuf_base, sizeof(VECTOR_T), VECTOR_RB_SIZE * sizeof(VECTOR_T));

	xPosition = yPosition = 0;

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_STEP_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_DIR_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_STEP_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_DIR_PIN);

	feedrate = 20;

	/* Init a Time for Motor Controller Use */
	Chip_TIMER_Init(LPC_TIMER32_0);

	/* Timer rate is system clock rate */
	timerFreq = Chip_Clock_GetSystemClockRate();

	/* Timer setup for match and interrupt at TICKRATE_HZ */
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, (timerFreq / 100));
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 1);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, (timerFreq / 100) * 2);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 1);
	Chip_TIMER_Enable(LPC_TIMER32_0);

	/* Enable timer interrupt */
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	NVIC_EnableIRQ(TIMER_32_0_IRQn);
	return;
}

void TIMER32_0_IRQHandler(void)
{
	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 0)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 0);
		processMoves();
	}else if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_STEP_PIN, 0);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_STEP_PIN, 0);
		Board_LED_Set(0, 0);
	}
}

uint8_t addVector(int16_t x, int16_t y, int8_t z){
	if(RingBuffer_IsFull(&vectorbuf))
		return 1;

	VECTOR_T tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;

	RingBuffer_Insert(&vectorbuf, &tmp);

	printf("Vector %d %d %d \n", x, y, z);

	return 0;
}

uint8_t moveAbsolutly(int32_t x, int32_t y){
	return moveRelativly(x - xPosition, y - yPosition, 0);
}

/* Move tool base on relative position */
/* no err = 0, else = 1*/
uint8_t moveRelativly(int32_t x, int32_t y, int8_t z){
	uint32_t i, error_acc;

	//Decide Direction of rotation
	int8_t xDirection = x < 0 ? -1 : 1;
	int8_t yDirection = y < 0 ? -1 : 1;

	//Take Absolute Value
	x = x < 0 ? x * (-1) : x;
	y = y < 0 ? y * (-1) : y;

	if(x == y){
	    for(i = 0; i < x; i++){
	      xPosition += xDirection;
	      yPosition += yDirection;
	      InsertMove(xDirection, yDirection, 0);
	    }
	}else if(x > y){
		error_acc = x >> 1;
	    for(i = 0; i < x; i++){
	    	xPosition += xDirection;
			error_acc -= y;
			if(error_acc < 0){
				yPosition += yDirection;
				error_acc += x;

				InsertMove(xDirection, yDirection, 0);
			}else{
				InsertMove(xDirection, 0, 0);
			}
	    }
	}else{
		error_acc = y >> 1;
		for(i = 0; i < y; i++){
            yPosition += yDirection;
			error_acc -= x;
			if(error_acc < 0){
				xPosition += xDirection;
				error_acc += y;

				InsertMove(xDirection, yDirection, 0);
			}else{
				InsertMove(0, yDirection, 0);
			}
		}
	}

	if(z)
		InsertMove(0, 0, z > 0 ? 1 : -1);

	return 0;
}

uint8_t bufferHasEnoughRoom(int32_t x, int32_t y, int8_t z){
	x = x < 0 ? x * (-1) : x;
	y = y < 0 ? y * (-1) : y;
	z = z < 0 ? z * (-1) : z;
	return (RingBuffer_GetFree(&movebuf) >= (MAX(MAX(x, y) , z) + 1));
}

void InsertMove(int8_t x, int8_t y, int8_t z){
	MOVE_T tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;

	RingBuffer_Insert(&movebuf, &tmp);

	return;
}

/* step X motor for one step */
void StepX(int8_t direction){
	direction = direction < 0 ? 0 : 1;

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_DIR_PIN, direction);

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_STEP_PIN, 1);
	Board_LED_Set(0, 1);
	return;
}

/* step Y motor for one step */
void StepY(int8_t direction){
	direction = direction < 0 ? 0 : 1;

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_DIR_PIN, direction);

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_STEP_PIN, 1);
	return;
}

void SetSpeed(uint16_t rate){
	feedrate = 200 / rate; //rate -> (mm/s)
}

void processMoves(void){
	if(RingBuffer_IsEmpty(&movebuf))
		return;

	MOVE_T aMove;
	RingBuffer_Pop(&movebuf, &aMove);

	if(aMove.x)
		StepX(aMove.x);
	if(aMove.y)
		StepY(aMove.y);

	return;
}

void processVectors(void){
	if(RingBuffer_IsEmpty(&vectorbuf))
		return;

	VECTOR_T* data = (VECTOR_T*)vectorbuf.data;

	if(!bufferHasEnoughRoom(data->x, data->y, data->z))
		return;

	VECTOR_T tmp;
	RingBuffer_Pop(&vectorbuf, &tmp);

	moveRelativly(tmp.x, tmp.y, tmp.z);

	return;
}


