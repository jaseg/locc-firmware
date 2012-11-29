/*
    c_locc: The all new LDAP-connected cybernetic lock
    Copyright 2012 jaseg <s@jaseg.de> & lynxis <lynxis@c-base.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <avr/io.h>

#define KEYPAD_COLS_INPUT       PIND
#define KEYPAD_COLS_PORT        PORTD
#define KEYPAD_COLS_FIRST_PIN   0

extern uint8_t matrix_selector;

void keypad_setup(void);
//Called to scan the next row of the keypad. Returns the highest pressed digit (0x00-0x09), with A-D being mapped to 0xA-0xD
uint8_t keypad_scan(void);

#endif//__KEYPAD_H__
