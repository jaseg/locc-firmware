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
#include "dial.h"

void dial_setup(){
	IMPULSE_PORT |= (1<<IMPULSE_PIN);
	DIAL_PORT |= (1<<DIAL_PIN);
	HANGUP_PORT |= (1<<HANGUP_PIN);
}

char dial_scan(){
	static uint8_t dial_counter = 0;
	static uint8_t state_impulse = 0;

	int8_t rv = 0;

	if(state_impulse <= 1){
		if(IMPULSE_INPUT & (1<<IMPULSE_PIN)){
			if(state_impulse == 0){
				state_impulse = 0x40;
				dial_counter++;
			}
		}else{
			state_impulse = 0;
		}
	}else{
		state_impulse--;
	}
	static uint8_t state_dialing = 0;
	if(state_dialing <= 1){
		if(DIAL_INPUT & (1<<DIAL_PIN)){
			if(state_dialing == 0){
				state_dialing = 0x80;
				if(dial_counter >= 10){
					dial_counter = 0;
				}
				rv = '0'+dial_counter;
				dial_counter = 0;
			}
		}else{
			state_dialing = 0;
		}
	}else{
		state_dialing--;
	}
	static uint8_t state_hangup = 0;
	if(state_hangup <= 2){
		if(HANGUP_INPUT & (1<<HANGUP_PIN)){
			if(state_hangup == 0){
				state_hangup = 0xFF;
				rv = 'H';
			}else{
				state_hangup = 1;
			}
		}else{
			if(state_hangup == 1){
				state_hangup = 0xFF;
				rv = 'h';
			}else{
				state_hangup = 0;
			}
		}
	}else{
		state_hangup--;
	}
	
	return rv;
}


