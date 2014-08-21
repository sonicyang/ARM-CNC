from uart_protocol import *
import sys


def main():
    
    UART_Init()

    SystemRunning = 1

    command = ""
    while(command != "stop"):
        command = input(">")
        if(command == "X"):
            command = input("length?")
            UART_Send_MOVE(int(command), 0)
        elif(command == "ECHO"):
            command = input("Number?")
            UART_Send_ECHO(int(command))

    UART_DeInit()
    SystemRunning = 0
    
if __name__ == "__main__":
    main()
