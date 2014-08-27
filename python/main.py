from uart_protocol import *
from gcodeinter import *
import sys, getopt
import pdb

def printHelp():
    print("List of Aviliable Commands:")
    print("  -h, --help\t\tPrint this help")
    print("  -l, --list\t\tList aviliable serial ports")
    print("  -f, --file\t\tExcute G-code File Passed in")
    print("  -i\t\t\tEnter Interactive G-code Shell")

def interactiveShell():
    try:

        print("Welcome to the Interactive Shell")
        print("Please choose serial port the CNC machine is connected to\n")
        print("List of Serial Ports : ")
        for name in UART_ListPorts():
            print("  " + name)

        name = input("\nPort Name : ")
        
        print("\nList of Aviliable Commands:")
        print("  G00 G01    Linear Movement")
        print("  G02 G03    Circle Movement")
        print("  M03        Activate Iron")
        print("  M05        Deactivate Iron")
        print("  G20        Set Unit in inchs")
        print("  G21        Set Unit in mm")
        print("  stop       Quit Interactive Shell")
        print("\n")

        UART_Init(name)

        command = ""
        while(command != "stop"):
            command = input(">")
            if(command != "ECHO"):
                ExcuteGCode(command)
            else:
                UART_Send_ECHO(0x1A)

        UART_DeInit()
    except KeyboardInterrupt:
        pass

def limitMovement(x, y):
    pass

def main(argv):
    if len(argv) < 1:
        print("Error Option Needed\n")
        printHelp()
        sys.exit(1)

    try:
        opts, args = getopt.getopt(argv, "hlif:", ["help", "list", "file="])
    except getopt.GetoptError:
        print("Error unknown option, or missing argument\n")
        printHelp()
        sys.exit(1)
    
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            printHelp()
        elif opt in ("-l", "--list"):
            print("List of Serial Ports : ")
            for name in UART_ListPorts():
                print("  " + name)
        elif opt in ("-i"):
            interactiveShell()
        elif opt in ("-f", "--file"):
            name = input("\nPort Name : ")

            UART_Init(name)
            f = open(arg)
            for line in f.readlines():
                ExcuteGCode(line)


if __name__ == "__main__":
    main(sys.argv[1:])
