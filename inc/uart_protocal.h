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
#define	DATA_SIZE 4
#define PACKET_SIZE 13

RINGBUFF_T txbuf, rxbuf, tpktbuf;

uint8_t	txbuf_base[UART_RB_SIZE], rxbuf_base[UART_RB_SIZE], tpktbuf_base[UART_TRANSMIT_RB_SIZE];

typedef struct {
	uint8_t 	magicNumber[3];	//Indicates packet 0xAA 0xAA 0xAA
	uint8_t		command;
	int16_t 	data[DATA_SIZE];
	uint8_t		checksum;
} PACKET_T;

enum{
	IDLE = 1,
	MOVE,
	SPEEDSET,
	DEACTIVE,
	ACK,
	NAK,
	TAL,
	ECHO,
	SPEED
};

void UART_IRQHandler(void);

void UART_init(void);

void generateHeader(PACKET_T*);
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

