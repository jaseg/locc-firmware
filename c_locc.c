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
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>

#include "Descriptors.h"
#include <LUFA/Version.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/USB/Class/CDCClass.h>

#include "locc.h"
#include "keypad.h"

void setup(void);
void loop(void);
void usb_putc(char c);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
    {
        .Config =
            {
                .ControlInterfaceNumber   = 0,
                .DataINEndpoint           =
                    {
                        .Address          = CDC_TX_EPADDR,
                        .Size             = CDC_TXRX_EPSIZE,
                        .Banks            = 1,
                    },
                .DataOUTEndpoint =
                    {
                        .Address          = CDC_RX_EPADDR,
                        .Size             = CDC_TXRX_EPSIZE,
                        .Banks            = 1,
                    },
                .NotificationEndpoint =
                    {
                        .Address          = CDC_NOTIFICATION_EPADDR,
                        .Size             = CDC_NOTIFICATION_EPSIZE,
                        .Banks            = 1,
                    },
            },
    };

void usb_putc(char c){
    //CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c);
}

int main(void){
    setup();
    for(;;) loop();
}

void setup(){
    //Disable watchdog if enabled by bootloader/fuses FIXME is the actually necessary? I ctrlc-ctrlv'ed this from the lufa sources
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    //output setup
	//DDRC |= 0x07;
    //PD2 hangup
    //PD3 impulse
    //PD4 dialing
    //PORTD |= 0x1C;
    //DDRB |= 0x20; //??? FIXME
	
    loccSetup();
    keypad_setup();
	clock_prescale_set(clock_div_1);
    USB_Init();
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
    int receive_status = -1;
    do{ //Always empty the receive buffer since there are _delay_xxs in the following code and thus this might not run all that often.
        receive_status = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        char c = receive_status&0xFF;
		
        /* Command set:
         * [command]\n
         * Commands:
         * o - open lock
         * l[a][b] set led [a] to [b] (a, b: ascii chars, b: 0 - off, 1 - on)
         */
        if(receive_status >= 0){
			CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c);
				  
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

    }while(receive_status >= 0);

	/*
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
                usb_putc('0'+dial_counter);
                dial_counter = 0;
                usb_putc('\n');
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
                usb_putc('h');
                usb_putc('\n');
            }
            state_hangup = 1;
        }else{
            if(state_hangup == 1){
                state_hangup = 0xFF;
                usb_putc('i');
                usb_putc('\n');
            }
            state_hangup = 0;
        }
    }else{
        state_hangup--;
    }
	*/

    //Keypad stuff
    uint8_t pressed_key = keypad_scan();
    if(pressed_key < 0xA)
        usb_putc('0'+pressed_key);
    else
        usb_putc('0'-0xA+pressed_key);

	//USB stuff
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();

    //_delay_us(255);
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}
