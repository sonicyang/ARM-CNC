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
        self.data = [0] * 3 
        self.checksum = 0

PACKET_FMT = "<3BB3hB"   #packet formant
PACKET_SIZE = 11 

IDLE    = 1
MOVE    = 2   
ACTIVE  = 3
DEACTIVE= 4
ACK     = 5
NAK     = 6
TAL     = 7
ECHO    = 8

MAX_TRY_COUNT = 2000000000000

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
    
    while(not (txbuf.empty())):
        pass

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
    chksum = (packet.command \
            + packet.data[0] + packet.data[1] + packet.data[2]) 
    return (((chksum >> 8) + chksum)) & 0xFF

def UARTTranciver(portname):
    global SystemRunning
    global txbuf
    global rxbuf

    Timeout_Count = 0
    Timeout_Amount = 0.1

    UART_ACK_FLAG = 1
    UART_NAK_FLAG = 0
    UART_TIMEOUT_FLAG = 0
    UART_TAL_FLAG = 0
    UART_ACK_PENDING = 0   

    repeat_buf = PACKET_T()
    target_time = time.time()

    ser = serial.Serial(portname, 115200, timeout=0)
    
    while(SystemRunning or UART_ACK_PENDING):

        if(UART_TAL_FLAG):
            time.sleep(2)

        if(UART_NAK_FLAG or UART_TIMEOUT_FLAG or UART_TAL_FLAG):
            ser.write(struct.pack(PACKET_FMT, 0xAA, 0xAA, 0xAA, repeat_buf.command, repeat_buf.data[0], repeat_buf.data[1], repeat_buf.data[2], repeat_buf.checksum))
            
            print("ReSend", repeat_buf.command, repeat_buf.data[0], repeat_buf.data[1])
            
            UART_ACK_PENDING = 1
            target_time = time.time() + Timeout_Amount

            UART_NAK_FLAG = UART_TIMEOUT_FLAG = UART_TAL_FLAG = 0

        if((not txbuf.empty()) and (UART_ACK_FLAG)):
            packet = txbuf.get();
            packet.checksum = calculateCheckSum(packet)
            
            repeat_buf = packet

            print("Send", packet.command, packet.data[0], packet.data[1])

            ser.write(struct.pack(PACKET_FMT, 0xAA, 0xAA, 0xAA, repeat_buf.command, repeat_buf.data[0], repeat_buf.data[1], repeat_buf.data[2], repeat_buf.checksum))
            ser.flush()

            UART_ACK_PENDING = 1
            target_time = time.time() + Timeout_Amount
            UART_ACK_FLAG = 0 

        if(not rxbuf.full()):
            raw_data = ser.read(PACKET_SIZE);
            for by in raw_data:
                rxbuf.put(by);

        while(rxbuf.qsize() >= PACKET_SIZE):
                        
            if((rxbuf.queue[0] != 0xAA) or (rxbuf.queue[1] != 0xAA) or (rxbuf.queue[2] != 0xAA)):
                rxbuf.get()
                continue

            raw_data = bytearray()
            for i in range(0, PACKET_SIZE):
                raw_data.append(rxbuf.get())

            packet = PACKET_T()
            packet.magicNumber[0], packet.magicNumber[1], packet.magicNumber[2],\
                    packet.command, packet.data[0], packet.data[1], \
                    packet.data[2],\
                    packet.checksum = struct.unpack(PACKET_FMT, raw_data);
            
            if(calculateCheckSum(packet) != packet.checksum):
                continue

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
            else:
                print("TRASH!")
        
        if(((time.time() >= target_time)) and UART_ACK_PENDING):
            UART_TIMEOUT_FLAG = 1
            Timeout_Count += 1
            if(Timeout_Count >= MAX_TRY_COUNT):
                raise Exception("Connection Timeout, Disconnected?")
        elif(not UART_ACK_PENDING):
            Timeout_Count = 0

        time.sleep(0)
