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

import sys, time, serial, threading

ser = serial.Serial('/dev/serial/by-id/usb-lynxis_jaseg_c-lab_locc_system_6493534313335191E052-if00', 115200)

#ser.write(b'\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nALLHAILDISCORDIAFOOBAR23PONIES!!\n');

def print_serial():
	while True:
		sys.stdout.write(str(ser.read(1))[2:-1])
		sys.stdout.flush()
printer = threading.Thread(target=print_serial)
printer.daemon = True
printer.start()

while True:
	line = sys.stdin.readline()
	if line == 'o\n':
		ser.write(b'o\n')
		#print('SENT COMMAND')

