/*
===============================================================================
 Name        : ARM-CNC.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include "uart_protocal.h"
#include "motorController.h"
#include "timer.h"

uint32_t UART_REVEICE_FLAG, UART_TRANSMIT_FLAG, VECTOR_PROCESS_FLAG;

int main(void) {

    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);

    SysTick_Init();

    UART_init();
    motorControllerInit();

    startTimer(20, &UART_REVEICE_FLAG);
    startTimer(30, &UART_TRANSMIT_FLAG);
    startTimer(15, &VECTOR_PROCESS_FLAG);

    while(1) {
    	if(UART_REVEICE_FLAG){
    		processUART_Receive();
    		startTimer(20, &UART_REVEICE_FLAG);
    	}

    	if(UART_TRANSMIT_FLAG){
    		processUART_Transmit();
    		startTimer(20, &UART_TRANSMIT_FLAG);
    	}

    	if(VECTOR_PROCESS_FLAG){
    		processVectors();
    		startTimer(1, &VECTOR_PROCESS_FLAG);
    	}

    	if(!(UART_REVEICE_FLAG || UART_TRANSMIT_FLAG|| VECTOR_PROCESS_FLAG)){
    		processMoves();
    	}

    }
    return 0 ;
}
