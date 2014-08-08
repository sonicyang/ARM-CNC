/*
 * uart_protocal.h
 *
 *  Created on: 2014/8/5
 *      Author: sonicyang
 */

#ifndef UART_PROTOCAL_H_
#define UART_PROTOCAL_H_

#include "board.h"
#include "uart_13xx.h"
#include "ring_buffer.h"

#define UART_RB_SIZE 256
#define UART_TRANSMIT_RB_SIZE 16
#define	DATA_SIZE 16

RINGBUFF_T txbuf, rxbuf, tpktbuf;

uint8_t	txbuf_base[UART_RB_SIZE], rxbuf_base[UART_RB_SIZE], tpktbuf_base[UART_TRANSMIT_RB_SIZE];

typedef struct {
	uint8_t 	magicNumber[3];	//Indicates packet 0xAA 0xAA 0xAA
	uint8_t		transmissionNumber;
	uint8_t 	data[DATA_SIZE];
	uint8_t		checksum;
} PACKET_T;

typedef struct {
	uint16_t command;
	int16_t	p1;
	int16_t	p2;
	int16_t	p3;
	int16_t	p4;
	int16_t	p5;
	int16_t	p6;
	int16_t	p7;
} COMMAND_CAST_T;	//size should be identical to DATA_SIZE

enum{
	IDLE = 1,
	MOVE,
	ACTIVE,
	DEACTIVE,
	ACK,
	NAK,
	TAL,
	ECHO
};

void UART_IRQHandler(void);

void UART_init(void);

uint8_t isChecksumVaild(PACKET_T*);

void generateCheckSum(PACKET_T*);

uint8_t isCmdStart(void);

//void SendPacketWrapper(PACKET_T* pak);
void SendACK(void);
void SendNAK(void);
void SendTAL(void);

void processUART_Receive(void);
void processUART_Transmit(void);

#endif /* UART_PROTOCAL_H_ */

