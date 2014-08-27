import struct
from enum import Enum
import serial
import queue
from threading import Thread
import time
import sys
import glob
import pdb

class PACKET_T:
    def __init__(self):
        self.magicNumber = [0xAA, 0xAA, 0xAA]
        self.transmissionNumber = 0
        self.command = 0
        self.data = [0] * 7 
        self.checksum = 0

PACKET_FMT = "<3BBH7hB"   #packet formant
PACKET_SIZE = 21

IDLE    = 1
MOVE    = 2   
ACTIVE  = 3
DEACTIVE= 4
ACK     = 5
NAK     = 6
TAL     = 7
ECHO    = 8

MAX_TRY_COUNT = 20

global SystemRunning

global txbuf
global rxbuf

global UARTTranciverThread

def UART_ListPorts():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    if sys.platform.startswith('win'):
        ports = ['COM' + str(i + 1) for i in range(256)]

    elif sys.platform.startswith('linux'):
        # this is to exclude your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')

    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')

    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def UART_Init(portname):
    global UARTTranciverThread
    global SystemRunning
    global txbuf
    global rxbuf
   
    txbuf = queue.Queue(256)
    rxbuf = queue.Queue(256)

    SystemRunning = 1

    UARTTranciverThread = Thread(target = UARTTranciver, args=(portname,))
    UARTTranciverThread.start()

def UART_DeInit():
    global SystemRunning
    SystemRunning = 0
    UARTTranciverThread.join()

def UART_Send_ECHO(num):
    global txbuf

    packet = PACKET_T()
    packet.command = ECHO 
    packet.data[0]  = num
    
    txbuf.put(packet)

def UART_Send_MOVE(x, y, z):
    global txbuf

    packet = PACKET_T()
    packet.command = MOVE 
    packet.data[0] = x
    packet.data[1] = y
    packet.data[2] = z
    
    txbuf.put(packet)

def UART_Send_ACTIVATE():
    global txbuf

    packet = PACKET_T()
    packet.command = ACTIVE
    
    txbuf.put(packet)

def UART_Send_DEACTIVATE():
    global txbuf

    packet = PACKET_T()
    packet.command = DEACTIVE
    
    txbuf.put(packet)
def calculateCheckSum(packet):
    chksum = (packet.transmissionNumber + packet.command \
            + packet.data[0] + packet.data[1] + packet.data[2] + packet.data[3] \
            + packet.data[4] +  packet.data[5] +  packet.data[6])
    return (((chksum >> 8) + chksum)) & 0xFF

def UARTTranciver(portname):
    global SystemRunning
    global txbuf
    global rxbuf

    Timeout_Count = 0

    UART_ACK_FLAG = 1
    UART_NAK_FLAG = 0
    UART_TIMEOUT_FLAG = 0
    UART_TAL_FLAG = 0
    UART_ACK_PENDING = 0   

    repeat_buf = PACKET_T()
    target_time = time.time()

    ser = serial.Serial(portname, 115200, timeout=0.2) # 200ms
    
    while(SystemRunning):

        if(UART_TAL_FLAG):
            time.sleep(2)

        if(UART_NAK_FLAG or UART_TIMEOUT_FLAG or UART_TAL_FLAG):
            ser.write(struct.pack(PACKET_FMT, 0xAA, 0xAA, 0xAA, repeat_buf.transmissionNumber,\
                    repeat_buf.command, repeat_buf.data[0], repeat_buf.data[1], repeat_buf.data[2], repeat_buf.data[3], \
                    repeat_buf.data[4], repeat_buf.data[5], repeat_buf.data[6], repeat_buf.checksum))

            
            UART_ACK_PENDING = 1
            target_time = time.time() + 0.5

            UART_NAK_FLAG = UART_TIMEOUT_FLAG = UART_TAL_FLAG = 0

        if((not txbuf.empty()) and (UART_ACK_FLAG)):
            packet = txbuf.get();
            packet.checksum = calculateCheckSum(packet)
            
            print("Send", packet.command)

            ser.write(struct.pack(PACKET_FMT, 0xAA, 0xAA, 0xAA, packet.transmissionNumber,\
                    packet.command, packet.data[0], packet.data[1], packet.data[2], packet.data[3], \
                    packet.data[4], packet.data[5], packet.data[6], packet.checksum))

            repeat_buf = packet

            UART_ACK_PENDING = 1
            target_time = time.time() + 0.5

            UART_ACK_FLAG = 0

        if(not rxbuf.full()):
            raw_data = ser.read(PACKET_SIZE);
            if(len(raw_data) >= 21):

                packet = PACKET_T()
                packet.magicNumber[0], packet.magicNumber[1], packet.magicNumber[2],\
                        packet.transmissionNumber, packet.command, packet.data[0], packet.data[1], \
                        packet.data[2], packet.data[3], packet.data[4], packet.data[5], packet.data[6],\
                        packet.checksum = struct.unpack(PACKET_FMT, raw_data);
                
                print(packet.command)

                if(packet.command == ACK):
                    print("GOT ACK")
                    UART_ACK_FLAG = 1
                    UART_ACK_PENDING = 0
                elif(packet.command == NAK):
                    print("GOT NAK")
                    UART_NAK_FLAG = 1
                    UART_ACK_PENDING = 0
                elif(packet.command == TAL):
                    print("GOT TAL")
                    UART_TAL_FLAG = 1
                    UART_ACK_PENDING = 0
        
        if(((time.time() >= target_time)) and UART_ACK_PENDING):
            UART_TIMEOUT_FLAG = 1
            Timeout_Count += 1
            if(Timeout_Count >= MAX_TRY_COUNT):
                raise Exception("Connection Timeout, Disconnected?")
        elif(not UART_ACK_PENDING):
            Timeout_Count = 0

        time.sleep(0)
