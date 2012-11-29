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

#ifndef __DIAL_H__
#define __DIAL_H__

//Rotary phone stuff

#define IMPULSE_INPUT	PINC
#define IMPULSE_PORT	PORTC
#define IMPULSE_PIN		6
#define HANGUP_INPUT	PINC
#define HANGUP_PORT		PORTC
#define HANGUP_PIN		5
#define DIAL_INPUT		PINC
#define DIAL_PORT		PORTC
#define DIAL_PIN		4

void dial_setup(void);
char dial_scan(void);

#endif//__DIAL_H__
