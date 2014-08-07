import struct
from enum import Enum
import serial

PACKET_T = "<3BBH7hB"   #packet formant

IDLE    = 1
MOVE    = 2   
ACTIVE  = 3
DEACTIVE= 4
ACK     = 5
NAK     = 6
TAL     = 7
ECHO    = 8


def main():

    ser = serial.Serial("/dev/tty.SLAB_USBtoUART", 115200, timeout=None);

    chksum = (0xAA + 0xAA + 0xAA + ECHO)
    chksum = (((chksum >> 8) + chksum) ^ 0xFF) & 0xFF
    packet = struct.pack(PACKET_T, 0xAA, 0xAA, 0xAA, 0x00, ECHO, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, chksum)
    
    while(True):
        ser.write(packet)
        raw_data = ser.read(21);

        m1, m2, m3, num, cmd, p1, p2, p3, p4, p5, p6, p7, chk = struct.unpack(PACKET_T, raw_data);

        print(cmd);
main()
