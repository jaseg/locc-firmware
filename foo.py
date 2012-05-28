#!/usr/bin/env python
import serial

ser = serial.Serial("/dev/ttyUSB0", 9600)

while True:
    char = ser.read()
    print char
