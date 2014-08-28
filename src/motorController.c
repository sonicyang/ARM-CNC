/*
 * motorController.c
 *
 *  Created on: 2014/8/5
 *      Author: sonicyang
 */

#include "board.h"
#include "motorController.h"
#include "ring_buffer.h"
#include "i2c.h"

#define Pressure 300

#define SPEED_100KHZ         100000

static I2C_ID_T i2cDev = I2C0;
const uint8_t pumpAddr = 0xA0;

extern volatile uint32_t I2CCount;
extern volatile uint8_t I2CMasterBuffer[BUFSIZE];
extern volatile uint8_t I2CSlaveBuffer[BUFSIZE];
extern volatile uint32_t I2CMasterState;
extern volatile uint32_t I2CReadLength, I2CWriteLength;

void motorControllerInit(void){
	uint32_t timerFreq;

	RingBuffer_Init(&movebuf, movebuf_base, sizeof(MOVE_T), MOVE_RB_SIZE);
	RingBuffer_Init(&vectorbuf, vectorbuf_base, sizeof(VECTOR_T), VECTOR_RB_SIZE * sizeof(VECTOR_T));

	xPosition = yPosition = 0;

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_STEP_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_X_DIR_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_STEP_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, MOTOR_PORT, MOTOR_Y_DIR_PIN);

	if ( I2CInit( (uint32_t)I2CMASTER ) == FALSE )	/* initialize I2c */
	  {
		while ( 1 );				/* Fatal error */
	  }

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
	NVIC_SetPriority(TIMER_32_0_IRQn, 32);
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

	//printf("Vector %d %d %d \n", x, y, z);

	return 0;
}

uint8_t moveAbsolutly(int32_t x, int32_t y){
	return moveRelativly(x - xPosition, y - yPosition, 0);
}

/* Move tool base on relative position */
/* no err = 0, else = 1*/
uint8_t moveRelativly(int32_t x, int32_t y, int8_t z){
	int32_t i, error_acc;

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
		error_acc = x / 2;
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
		error_acc = y / 2;
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
	static  I2C_XFER_T xfer;
	MOVE_T aMove;

	if(RingBuffer_IsEmpty(&movebuf))
		return;

	RingBuffer_Pop(&movebuf, &aMove);

	if(aMove.x)
		StepX(aMove.x);
	if(aMove.y)
		StepY(aMove.y);

	if(aMove.z){
		if(aMove.z > 0){
			i2c_pump(&xfer, 1);
			//Chip_I2C_MasterSend(i2cDev, xfer.slaveAddr, xfer.txBuff, xfer.txSz);
		}else{
			i2c_pump(&xfer, 0);
			//Chip_I2C_MasterSend(i2cDev, xfer.slaveAddr, xfer.txBuff, xfer.txSz);
		}
	}


	return;
}

void processVectors(void){
	if(RingBuffer_IsEmpty(&vectorbuf))
		return;

	uint8_t i = RB_INDT(&vectorbuf);
	VECTOR_T* data = (VECTOR_T*)vectorbuf.data;

	if(!bufferHasEnoughRoom(data[i].x, data[i].y, data[i].z))
		return;


	VECTOR_T tmp;
	RingBuffer_Pop(&vectorbuf, &tmp);

	moveRelativly(tmp.x, tmp.y, tmp.z);

	return;
}

void i2c_pump(I2C_XFER_T *xfer, uint8_t ops)
{
	I2CWriteLength = 4;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = pumpAddr;
	I2CMasterBuffer[1] = 0x62;		/* address */
	I2CMasterBuffer[2] = Pressure >> 8;
	I2CMasterBuffer[3] = Pressure & 0xFF;
	I2CEngine();
	/*
	xfer->slaveAddr = pumpAddr;
	xfer->rxBuff = 0;
	xfer->txBuff = 0;
	xfer->txSz = 0;
	xfer->rxSz = 0;

	if (ops == 1) {
		i2cbuffer[0][0] = 0x62;
		i2cbuffer[0][1] = Pressure >> 8;
		i2cbuffer[0][2] = Pressure & 0xFF;
		xfer->txSz = 3;
		xfer->txBuff = i2cbuffer[0];
	}

	if (ops == 2) {
		i2cbuffer[0][0] = 0x62;
		i2cbuffer[0][1] = 0 >> 8;
		i2cbuffer[0][2] = 0 & 0xFF;
		xfer->txSz = 3;
		xfer->txBuff = i2cbuffer[0];
	}
	*/
}


