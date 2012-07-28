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

void keypad_setup(){
    KEYPAD_ROWS_DDR |= 0xF<<KEYPAD_ROWS_FIRST_PIN;
    KEYPAD_COLS_PORT |= 0xF<<KEYPAD_COLS_FIRST_PIN;
}

//FIXME debouncing
uint8_t keypad_scan(){
    static uint8_t row = 0;
    static uint8_t debouncing[4][4];
    KEYPAD_ROWS_PORT |= (1<<KEYPAD_ROWS_FIRST_PIN)<<row;
    uint8_t row_state = KEYPAD_COLS_INPUT>>KEYPAD_COLS_FIRST_PIN;
    uint8_t ret = 0;
    //FIXME due to a lack of information about the keypad, this does not yet include the decoding logic mapping these arbitrary numbers to pressed keys
    //TODO check whether *that* debouncing actually works
    //TODO currently, this only handles key presses, key releases are ignored
    for(uint8_t i=0; i<4; i++){ //Could loop unrolling make sense here? At least the shifts would then be static
        if(debouncing[row][i] <= 1){
            if(row_state & (1<<i)){
                if(debouncing[row][i] == 0){
                    ret = 0;
                    debouncing[row][i] = 0x40;
                }
            }else{
                debouncing[row][i] = 0;
            }
        }else{
            debouncing[row][i]--;
        }
    }
    ret |= row<<2;
    row++;
    row&=0xFC; //modulo 4
}
