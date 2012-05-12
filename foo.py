#!/usr/bin/env python
import serial
import time

ser = serial.Serial("/dev/ttyUSB0", 9600)
while True:
    ser.write("l01\n")
    time.sleep(1)
    ser.write("l00\n")
    time.sleep(1)

while True:
    line = ser.readline()
    print line
