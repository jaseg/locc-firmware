#!/usr/bin/env python
import serial
import time

ser = serial.Serial("/dev/ttyUSB0", 57600)

time.sleep(1)
ser.write("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nALLHAILDISCORDIAFOOBAR23PONIES!!\n");
ser.write("\no\n")
time.sleep(1)

while True:
    print ser.readline(),
