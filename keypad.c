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

#include <avr/io.h>
#include "keypad.h"

uint8_t matrix_selector = 0x10; //due to the position of the matrix row outputs on the shift register

void keypad_setup(){
	//enable internal pullups for the column inputs
    KEYPAD_COLS_PORT |= 0xF<<KEYPAD_COLS_FIRST_PIN;
}

//FIXME test debouncing
//CAUTION! This function relies on the caller to apply matrix_selector before the next call
uint8_t keypad_scan(){
    static uint8_t row = 0;
    static uint8_t debouncing[4][4];
    uint8_t row_state = ~(KEYPAD_COLS_INPUT>>KEYPAD_COLS_FIRST_PIN);
    uint8_t ret = 0xFF;
    //FIXME due to a lack of information about the keypad, this does not yet include the decoding logic mapping these arbitrary numbers to pressed keys
    //TODO check whether *that* debouncing actually works
    //TODO currently, this only handles key presses, key releases are ignored
    for(uint8_t i=0; i<4; i++){ //Could loop unrolling make sense here? At least the shifts would then be static
        if(debouncing[row][i] <= 1){
            if(row_state & (1<<i)){
                if(debouncing[row][i] == 0){
                    ret = i;
                    debouncing[row][i] = 0x40;
                }
            }else{
                debouncing[row][i] = 0;
            }
        }else{
            debouncing[row][i]--;
        }
    }
	if(ret != 0xFF)
		ret += row*4;

	matrix_selector <<= 1;
	row++;
	if(!matrix_selector){
		matrix_selector = 0x10;
		row = 0;
	}

	return ret;
}
