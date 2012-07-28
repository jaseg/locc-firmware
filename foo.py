#!/usr/bin/env python
#   c_locc: The all new LDAP-connected cybernetic lock
#   Copyright 2012 jaseg <s@jaseg.de> & lynxis <lynxis@c-base.org>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

import serial
import time

ser = serial.Serial("/dev/ttyUSB0", 57600)

time.sleep(1)
ser.write("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nALLHAILDISCORDIAFOOBAR23PONIES!!\n");
ser.write("\no\n")
time.sleep(1)

while True:
    print ser.readline(),
