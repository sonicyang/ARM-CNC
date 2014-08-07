/*
 * motorController.c
 *
 *  Created on: 2014/8/5
 *      Author: sonicyang
 */

#include "motorController.h"
#include "ring_buffer.h"

void motorControllerInit(void){
	RingBuffer_Init(&movebuf, movebuf_base, 1, MOVE_RB_SIZE);
	//Do GPIO Init Here;
	return;
}

void moveAbsolutly(int32_t x, int32_t y){
	moveRelativly(x - xPosition, y - yPosition);
}

/* Move tool base on relative position */
void moveRelativly(int32_t x, int32_t y){
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
	      InsertMove(xDirection, yDirection);
	    }
	}else if(x > y){
		error_acc = x >> 1;
	    for(i = 0; i < x; i++){
	    	xPosition += xDirection;
			error_acc -= y;
			if(error_acc < 0){
				yPosition += yDirection;
				error_acc += x;

				InsertMove(xDirection, yDirection);
			}else{
				InsertMove(xDirection, 0);
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

				InsertMove(xDirection, yDirection);
			}else{
				InsertMove(0, yDirection);
			}
		}
	}
}

void InsertMove(int8_t x, int8_t y){
	MOVE_T tmp;
	tmp.x = x;
	tmp.y = y;

	RingBuffer_Insert(&movebuf, &tmp);

	return;
}

/* step X motor for one step */
void StepX(uint8_t direction){

	xPosition += direction;
	return;
}

/* step Y motor for one step */
void StepY(uint8_t direction){

	yPosition += direction;
	return;
}

void processMoves(void){
	MOVE_T aMove;
	Ringbuffer_Pop(&movebuf, &aMove);

	if(aMove.x)
		StepX(aMove.x);
	if(aMove.y)
		StepY(aMove.y);

	return;
}

