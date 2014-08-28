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
        
        printGCodeHelp();

        UART_Init(name)

        command = ""
        while(command != "M100"):
            command = input(">")
            if(command == "ECHO"):
                UART_Send_ECHO(0x1A)
            else:
                ExcuteGCode(command)
        UART_DeInit()
    except KeyboardInterrupt:
        print("\nBye")
        try:
            UART_DeInit()
        except:
            pass

def execFile(filename):
    try:

        print("Excuting G-Code File")
        print("Please choose serial port the CNC machine is connected to\n")
        print("List of Serial Ports : ")
        for name in UART_ListPorts():
            print("  " + name)

        name = input("\nPort Name : ")
        
        UART_Init(name)
        f = open(filename)
        for line in f.readlines():
            ExcuteGCode(line)
        UART_DeInit()

    except KeyboardInterrupt:
        print("\nG-Code Excution Terminated")
        try:
            UART_DeInit()
        except:
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
            execFile(arg)


if __name__ == "__main__":
    main(sys.argv[1:])
