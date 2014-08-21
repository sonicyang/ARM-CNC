/*
 * motorController.h
 *
 *  Created on: 2014/8/5
 *      Author: sonicyang
 */

#ifndef MOTORCONTROLLER_H_
#define MOTORCONTROLLER_H_

#include <stdint.h>
#include "ring_buffer.h"

#define MOTOR_PORT	2

#define MOTOR_X_STEP_PIN	4
#define MOTOR_X_DIR_PIN 	5
#define MOTOR_Y_STEP_PIN	6
#define MOTOR_Y_DIR_PIN		7
#define MOVE_RB_SIZE 256

typedef struct {
	int8_t 	x;
	int8_t	y;
} MOVE_T;

RINGBUFF_T movebuf;

uint8_t	movebuf_base[MOVE_RB_SIZE];

uint32_t xPosition, yPosition;

uint32_t feedrate;

void motorControllerInit(void);

void StepX(int8_t direction);
void StepY(int8_t direction);

void EnableOutput(void);
void DisableOutput(void);

void SetSpeed(uint16_t rate);

uint8_t moveAbsolutly(int32_t x, int32_t y);
uint8_t moveRelativly(int32_t x, int32_t y);

uint8_t bufferHasEnoughRoom(int32_t x, int32_t y);

void InsertMove(int8_t x, int8_t y);
void processMoves(void);


#endif /* MOTORCONTROLLER_H_ */
