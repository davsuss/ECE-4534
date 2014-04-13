from webbrowser import Galeon
import serial
__author__ = 'David'
data = bytearray(6)


connected = False

#Constants:

#Straight distance: 0x08 = 1 foot
#Resolution 1.5 inch


#



while connected is False:
# Open serial port
    try:
        PORT = "/dev/tty.usbserial-A702ZKR0"
        ser = serial.Serial(PORT, 19200)
        connected = True
        print("Port Sucessfully opened\n")
    except serial.SerialException:
        print("Port Failed to open please try again\n")


def TurnLeft45():
    data[0] = 0x21
    data[1] = 0x22
    data[2] = 0x00
    data[3] = 0x04
    data[4] = 0x00
    data[5] = data[0] + data[1] + data[2] + data[3] + data[4]
    ser.write(data)

def TurnLeft90():
    data[0] = 0x21
    data[1] = 0x22
    data[2] = 0x00
    data[3] = 0x09
    data[4] = 0x00
    data[5] = data[0] + data[1] + data[2] + data[3] + data[4]
    ser.write(data)
def StraightFoot():
    data[0] = 0x21
    data[1] = 0x20
    data[2] = 0x00
    data[3] = 0xA0#0x08#0xA0
    data[4] = 0x00
    data[5] = data[0] + data[1] + data[2] + data[3] + data[4]
    ser.write(data)
def TurnRight90():
    data[0] = 0x21
    data[1] = 0x21
    data[2] = 0x00
    data[3] = 0x09
    data[4] = 0x00
    data[5] = data[0] + data[1] + data[2] + data[3] + data[4]
    ser.write(data)
def GetData():
    data[0] = 0x00
    data[1] = 0x00
    data[2] = 0x00
    data[3] = 0x00
    data[4] = 0x00
    data[5] = data[0] + data[1] + data[2] + data[3] + data[4]
    ser.write(data)
    while True:
        result = ser.read(64)
        if(len(result) > 0):
            print(result)


#def TurnLeft45():
#data[0]
#data[1]
#data[2]
#data[3]
#data[4]
#data[5]
#def TurnRight45():
#data[0]
#data[1]
#data[2]
#data[3]
#data[4]
#data[5]





if(__name__ == "__main__"):
   StraightFoot()


