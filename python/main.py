from uart_protocol import *

def main():
    
    UART_Init()

    SystemRunning = 1

    count  = 0 
    while(SystemRunning):
        UART_Send_ECHO(count)
        time.sleep(0.05)
        count += 1

    UART.DeInit()
    SystemRunning = 0
    

main()
