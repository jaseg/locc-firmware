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
#include <string.h>
#include <stdio.h>

#include "Descriptors.h"
#include "srsly/USB.h"
#include "srsly/CDCClass.h"

#include "locc.h"
#include "dial.h"
#include "keypad.h"

void setup(void);
void loop(void);
void set_led(uint8_t num, uint8_t val);
void shiftreg_out(void);
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

int main(void){
    setup();
    for(;;) loop();
}

static FILE USBSerialStream;

void setup(){
    //Disable watchdog if enabled by fuses
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    //setup spi
	//SS,MOSI,SCK
	DDRB |= 0x07;
	PORTB |= 0x01;
	//RCLK
	DDRD |= 0x10;
	PORTD |= 0x10;
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA); //spi data rate: f_clk/16 = 1MHz (it's only a few millimeters)

	dial_setup();
    loccSetup();
    keypad_setup();
	clock_prescale_set(clock_div_1);
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
    USB_Init();
    sei();
}

uint8_t led_states = 0x00;

//0<=num<3; val=0 => off val>0 => on
void set_led(uint8_t num, uint8_t val){
	if(num<3){
		led_states &= ~(1<<num);
		led_states |= (!val)<<num;
	}
}

void shiftreg_out(){
	//CAUTION!! This does *not* wait until the byte has been transmitted, thus it must only be called every so-and-so microseconds.
	SPDR = led_states; //led_states | matrix_selector;
	//while(!(SPSR&(1<<SPIF))){
		//She waves and opens a door back onto the piazza where her robot cat -- the alien's nightmare intruder in the DMZ -- sleeps, chasing superintelligent dream mice through multidimensional realities. 
	//}
	//toggle stc
	PORTD &= ~0x10;
	PORTD |= 0x10;
}

void loop(){ //one frame
	static uint8_t protocol_state = 0;
	static uint8_t cmd_target = 0;
	int16_t rb = -1;

	//CDC_Device_Flush(&VirtualSerial_CDC_Interface);
	//fputs("FooBar\n", &USBSerialStream);
    //do{ //Empty the receive buffer
        rb = fgetc(&USBSerialStream);//CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		
        /* Command set:
         * [command]\n
         * Commands:
         * o - open lock
         * l[a][b] set led [a] to [b] (a, b: ascii chars, b: 0 - off, 1 - on)
         */
        if(rb != EOF){
			char c = rb;
			//CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c);
			//fputc(c, &USBSerialStream);
			fputs("Foobar\n", &USBSerialStream);
				  
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
					set_led(cmd_target-'0', c-'0');
					protocol_state = 0;
					break;
			}
        }

    //}while(rb >= 0);

    //Keypad stuff
	/*
	uint8_t pressed_key = keypad_scan();
	if(pressed_key != 0xFF){
		if(pressed_key < 0xA){
			usb_putc('0'+pressed_key);
		}else{
			usb_putc('0'-0xA+pressed_key);
		}
		usb_putc('\n');
	}
	*/

	//output led and matrix driver signals via shift register
	//CAUTION! This must not be called more often than every like 8 microseconds.
	shiftreg_out();

	//rotary dial stuff
	uint8_t d = dial_scan();
	if(d & 0x40){
		//usb_putc('i');
		//usb_putc('\n');
	}else if(d & 0x80){
		//usb_putc('h');
		//usb_putc('\n');
	}else if(!(d & 0x10)){
		if(d&0xF < 10){
			//usb_putc('0'+d&0xF);
			//usb_putc('\n');
		}
	}
	/* FIXME debug code
	if(IMPULSE_INPUT & (1<<IMPULSE_PIN))
		usb_putc('I');
	else
		usb_putc('i');
	if(HANGUP_INPUT & (1<<HANGUP_PIN))
		usb_putc('H');
	else
		usb_putc('h');
	if(DIAL_INPUT & (1<<DIAL_PIN))
		usb_putc('D');
	else
		usb_putc('d');
	usb_putc('\n');
	*/

	//USB stuff
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();

    //_delay_us(255);
}

void usb_putc(char c){
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c);
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}
