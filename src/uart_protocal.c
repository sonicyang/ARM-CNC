/*
 * uart_protocal.c
 *
 *  Created on: 2014/8/5
 *      Author: sonicyang
 */

#include "uart_protocal.h"
#include "motorController.h"
#include "timer.h"
#include <stdio.h>

PACKET_T repeatBuf;
uint32_t UART_TIMEOUT_FLAG = 0;
uint32_t UART_ACK_FLAG = 0;
uint32_t UART_NAK_FLAG = 0;
uint32_t UART_TAL_FLAG = 0;

void UART_IRQHandler(void){
	Chip_UART_IRQRBHandler(LPC_USART, &rxbuf, &txbuf);
}


void UART_init(void){
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 115200);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&rxbuf, rxbuf_base, 1, UART_RB_SIZE);
	RingBuffer_Init(&txbuf, txbuf_base, 1, UART_RB_SIZE);
	RingBuffer_Init(&tpktbuf, tpktbuf_base, 1, UART_TRANSMIT_RB_SIZE);

	/* Enable receive data and line status interrupt, the transmit interrupt
	   is handled by the driver. */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);
}

void generateHeader(PACKET_T* pak){
	pak->magicNumber[0] = 0xAA;
	pak->magicNumber[1] = 0xAA;
	pak->magicNumber[2] = 0xAA;
	return;
}

uint8_t isChecksumVaild(PACKET_T* pak){
	uint8_t i;
	uint16_t chk = 0;

	chk += pak->transmissionNumber;
	chk += pak->command;
	for(i = 0; i < DATA_SIZE; i++)
		chk +=pak->data[i];

	chk = ((chk >> 8) + chk) & 0xFF;

	if(chk != pak->checksum)
		return FALSE;
	return TRUE;
}

void generateCheckSum(PACKET_T* pak){
	uint8_t i;
	uint16_t chk = 0;

	chk += pak->transmissionNumber;
	chk += pak->command;
	for(i = 0; i < DATA_SIZE; i++)
		chk +=pak->data[i];
	pak->checksum = ((chk >> 8) + chk);

	return;
}

uint8_t isPacketStart(void){
	uint8_t i = RB_INDT(&rxbuf);
	uint8_t* data = (uint8_t*)rxbuf.data;
	if(data[i] == 0xAA)
		if(data[i + 1] == 0xAA)
			if(data[i + 2] == 0xAA)
				return TRUE;

	return FALSE;
}

void SendACK(void){
	PACKET_T response;
	generateHeader(&response);
	response.command = ACK;
	generateCheckSum(&response);
	while(RingBuffer_GetFree(&txbuf) < sizeof(PACKET_T));	//Make Sure this is sent
	Chip_UART_SendRB(LPC_USART, &txbuf, &response, sizeof(PACKET_T));
}

void SendNAK(void){
	PACKET_T response;
	generateHeader(&response);
	response.command = NAK;
	generateCheckSum(&response);
	while(RingBuffer_GetFree(&txbuf) < sizeof(PACKET_T));	//Make Sure this is sent
	Chip_UART_SendRB(LPC_USART, &txbuf, &response, sizeof(PACKET_T));
}

void SendTAL(void){
	PACKET_T response;
	generateHeader(&response);
	response.command = TAL;
	generateCheckSum(&response);
	while(RingBuffer_GetFree(&txbuf) < sizeof(PACKET_T));	//Make Sure this is sent
	Chip_UART_SendRB(LPC_USART, &txbuf, &response, sizeof(PACKET_T));
}

void processUART_Receive(void){

	while(RingBuffer_GetCount(&rxbuf) >= sizeof(PACKET_T)){
		uint8_t trash;	//Popping trash away

		if(isPacketStart()){
			PACKET_T data;

			RingBuffer_PopMult(&rxbuf, &data, sizeof(PACKET_T));

			if(isChecksumVaild(&data)){
				switch(data.command){
					case IDLE:
						UART_TAL_FLAG = FALSE;
						break;
					case MOVE:
						if(moveAbsolutly(data.data[0], data.data[2]))
							SendTAL();
						else
							SendACK();
						break;
					case ACTIVE:

						SendACK();
						break;
					case DEACTIVE:

						SendACK();
						break;
					case ACK:
						UART_ACK_FLAG = TRUE;
						stopTimer(&UART_TIMEOUT_FLAG);
						break;
					case NAK:
						UART_NAK_FLAG = TRUE;
						stopTimer(&UART_TIMEOUT_FLAG);
						break;
					case TAL:
						UART_TAL_FLAG = TRUE;
						break;
					case ECHO:
						SendACK();
						printf("ECHO %d\n", data.data[0]);
						break;
				}
			}else{	//Check Sum Error
				SendNAK();
			}
		}else{
			RingBuffer_Pop(&rxbuf, &trash);
		}
	}

	return;
}

void processUART_Transmit(void){
	if(UART_TAL_FLAG)
		return;

	if(RingBuffer_GetFree(&txbuf) < sizeof(PACKET_T))
		return;

	if(UART_ACK_FLAG){
		if(RingBuffer_GetCount(&tpktbuf) > 0){
			RingBuffer_Pop(&tpktbuf, &repeatBuf);

			Chip_UART_SendRB(LPC_USART, &txbuf, &repeatBuf, sizeof(PACKET_T));

			startTimer(500, &UART_TIMEOUT_FLAG);	//500ms
			UART_ACK_FLAG = FALSE;
		}
		return;
	}

	if(UART_TIMEOUT_FLAG || UART_NAK_FLAG){
		Chip_UART_SendRB(LPC_USART, &txbuf, &repeatBuf, sizeof(PACKET_T));
		startTimer(500, &UART_TIMEOUT_FLAG);	//500ms
		UART_NAK_FLAG  = UART_TIMEOUT_FLAG = FALSE;
		return;
	}

	return;
}

