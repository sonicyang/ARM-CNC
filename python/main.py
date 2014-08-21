from uart_protocol import *
from gcodeinter import *
import sys, getopt
import pdb

def printHelp():
    print("List of Aviliable Commands:")
    print("  -h, --help\t\tPrint this help")
    print("  -l, --list\t\tList aviliable serial ports")

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
            ExcuteGCode(command)

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
        opts, args = getopt.getopt(argv, "hli", ["help", "list"])
    except getopt.GetoptError:
        print("Error unknown option\n")
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

if __name__ == "__main__":
    main(sys.argv[1:])
