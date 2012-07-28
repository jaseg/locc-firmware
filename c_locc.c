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
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "locc.h"
#include "keypad.h"

void setup(void);
void loop(void);

int main(void){
    setup();
    for(;;) loop();
}

void setup(){
    uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(57600, F_CPU));
    DDRD |= 0x02; //uart tx
    DDRC |= 0x07;
    //DDRD |= 0x01;
    //PD2 hangup
    //PD3 impulse
    //PD4 dialing
    PORTD |= 0x1C;
    DDRB |= 0x20; //???
    loccSetup();
    keypad_setup();
    sei();
}

int parseHex(char* buf){
    int result = 0;
    int len = 2;
    for(int i=0; i<len; i++){
        char c = buf[len-i];
        int v = 0;
        if(c>='0' && c<='9'){
            v=(c-'0');
        }else if(c>='a' && c<= 'f'){
            v=(c-'a'+10);
        }else if(c>='A' && c<= 'F'){
            v=(c-'A'+10);
        }
        result |= v<<(4*i);
    }
    return result;
}

uint8_t protocol_state = 0;
uint8_t cmd_target = 0;
uint8_t started = 32;
char* password = "ALLHAILDISCORDIAFOOBAR23PONIES!!";

void set_led(uint8_t num, uint8_t val){
    switch(num){
        case '0':
            if(val)
                PORTB |= 0x20;
            else
                PORTB &= 0xDF;
            break;
        case '1':
            if(val)
                PORTC |= 0x01;
            else
                PORTC &= 0xFE;
            break;
        case '2':
            if(val)
                PORTC |= 0x02;
            else
                PORTC &= 0xFD;
            break;
        case '3':
            if(val)
                PORTC |= 0x04;
            else
                PORTC &= 0xFB;
            break;
    }
}

void loop(){ //one frame
    uint16_t receive_status = 1;
    do{ //Always empty the receive buffer since there are _delay_xxs in the following code and thus this might not run all that often.
        receive_status = uart_getc();
        char c = receive_status&0xFF;
        receive_status &= 0xFF00;
        /* Command set:
         * [command]\n
         * Commands:
         * o - open lock
         * l[a][b] set led [a] to [b] (a, b: ascii chars, b: 0 - off, 1 - on)
         */
        if(!receive_status){
            if(started > 0){
                if(c == password[32-started]){
                    started--;
                }else{
                    started = 32;
                }
            }else{
                switch(protocol_state){
                    case 0:
                        if(c == '\n')
                            protocol_state = 1;
                        break;
                    case 1:
                        switch(c){
                            case 'o':
                                loccOpen();
                                protocol_state = 0;
                                break;
                            case 'l':
                                protocol_state = 2;
                                break;
                            case '\n':
                                break;
                            default:
                                protocol_state = 0;
                        }
                        break;
                    case 2:
                        cmd_target = c;
                        protocol_state = 3;
                        break;
                    case 3:
                        set_led(cmd_target, c-'0');
                        protocol_state = 0;
                        break;
                }
            }
        }
    }while(!receive_status || started > 0);

    //Rotary phone stuff
    static uint8_t dial_counter = 0;
    static uint8_t state_impulse = 0;
    if(state_impulse <= 1){
        if(PIND & 0x08){
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
        if(PIND & 0x10){
            if(state_dialing == 0){
                state_dialing = 0x80;
                if(dial_counter >= 10){
                    dial_counter = 0;
                }
                uart_putc('0'+dial_counter);
                dial_counter = 0;
                uart_putc('\n');
            }
        }else{
            state_dialing = 0;
        }
    }else{
        state_dialing--;
    }
    static uint8_t state_hangup = 0;
    if(state_hangup <= 2){
        if(PIND & 0x04){
            if(state_hangup == 0){
                state_hangup = 0xFF;
                uart_putc('h');
                uart_putc('\n');
            }
            state_hangup = 1;
        }else{
            if(state_hangup == 1){
                state_hangup = 0xFF;
                uart_putc('i');
                uart_putc('\n');
            }
            state_hangup = 0;
        }
    }else{
        state_hangup--;
    }

    //Keypad stuff
    uint8_t pressed_key = keypad_scan();
    if(pressed_key < 0xA)
        uart_putc('0'+pressed_key);
    else
        uart_putc('0'-0xA+pressed_key);
    _delay_us(255);
}
