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
#define VECTOR_RB_SIZE	30
#define MOVE_RB_SIZE 512

typedef struct {
	int16_t x;
	int16_t	y;
	int8_t z;
	uint8_t f;
} VECTOR_T;

typedef struct {
	int8_t 	x:	3;
	int8_t	y:	3;
	int8_t  z: 	2;
	uint8_t f;
} MOVE_T;

RINGBUFF_T movebuf;
RINGBUFF_T vectorbuf;

uint8_t	movebuf_base[MOVE_RB_SIZE];
VECTOR_T vectorbuf_base[VECTOR_RB_SIZE];
uint16_t vbufH, vbufT, vbufCount;


uint32_t xPosition, yPosition;

uint32_t feedrate;

void motorControllerInit(void);

void TIMER32_0_IRQHandler(void);

void StepX(int8_t direction);
void StepY(int8_t direction);

void EnableOutput(void);
void DisableOutput(void);

void SetSpeed(uint16_t rate);

uint8_t addVector(int16_t x, int16_t y, int8_t z, uint8_t f);

uint8_t moveAbsolutly(int32_t x, int32_t y);
uint8_t moveRelativly(int32_t x, int32_t y, int8_t z);

uint8_t bufferHasEnoughRoom(int32_t x, int32_t y, int8_t z);

void InsertMove(int8_t x, int8_t y, int8_t z);
void processMoves(void);
void processVectors(void);

void i2c_pump(uint8_t ops);


#endif /* MOTORCONTROLLER_H_ */
