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

#include "c_locc.h"
#include "locc.h"
#include "dial.h"
// #include "keypad.h"

#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Peripheral/Serial.h>


void setup(void);
void loop(void);
void set_led(uint8_t num, uint8_t val);
void shiftreg_out(void);
void usb_putc(char c);

/** Circular buffer to hold data from the host before it is sent to the device via the serial port. */
static RingBuffer_t USB_input_Buffer;

/** Underlying data buffer for \ref USB_input_Buffer, where the stored bytes are located. */
static uint8_t      USB_input_Buffer_Data[128];

/** Circular buffer to hold data from the serial port before it is sent to the host. */
static RingBuffer_t USB_output_Buffer;

/** Underlying data buffer for \ref USB_output_Buffer, where the stored bytes are located. */
static uint8_t      USB_output_Buffer_Data[128];

static bool is_locking = false;

enum protocol_state { WAIT_FOR_NEWLINE, WAIT_FOR_CMD_CHAR, WAIT_FOR_LED_NUMBER, WAIT_FOR_LED_VALUE };

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
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

void setup(){
    //Disable watchdog if enabled by fuses
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    
    // we want to run at 16mhz
	clock_prescale_set(clock_div_1);

    //setup spi
	//SS,MOSI,SCK
	DDRB |= 0x07;
	PORTB |= 0x01;
	//RCLK
	DDRD |= 0x10;
	PORTD |= 0x10;
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA); //spi data rate: f_clk/16 = 1MHz (it's only a few millimeters)

    USB_Init();
    RingBuffer_InitBuffer(&USB_input_Buffer, USB_input_Buffer_Data, sizeof(USB_input_Buffer_Data));
    RingBuffer_InitBuffer(&USB_output_Buffer, USB_output_Buffer_Data, sizeof(USB_output_Buffer_Data));

	//dial_setup();
    loccSetup();
    //keypad_setup();

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
	while(!(SPSR&(1<<SPIF))){
		//She waves and opens a door back onto the piazza where her robot cat -- the alien's nightmare intruder in the DMZ -- sleeps, chasing superintelligent dream mice through multidimensional realities. 
	}
	led_states ^= 0x07;
	PORTD &= ~0x10;
	PORTD |= 0x10;
}

void loop() { //one frame

	int16_t receive_status = -1;
	int command = 0;

    receive_status = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    Endpoint_SelectEndpoint(VirtualSerial_CDC_Interface.Config.DataINEndpoint.Address);
    char c = (receive_status & 0xFF);

    /* Command set:
     * [command]\n
     * Commands:
     * o - open lock
     * l[a][b] set led [a] to [b] (a, b: ascii chars, b: 0 - off, 1 - on)
     */
    if(!(receive_status < 0)) {
       	if (!is_locking) {
			command = handle_user_input(c);
	   	}
		else {
			// ignore any input
		}
    }
	if (command == 1) {
		is_locking = true;
	}

	if (is_locking) {
		is_locking = do_next_locc_step();
	}
	
	//output led and matrix driver signals via shift register
	//CAUTION! This must not be called more often than every like 8 microseconds.
  	_delay_us(8);
	shiftreg_out();

	//USB stuff
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();

    //_delay_us(255);
}

int handle_user_input(char c) {
	static enum protocol_state p_state = WAIT_FOR_NEWLINE;
	static uint8_t cmd_target = 0;
	int command = 0;
	
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c);
	if(c == '\n' || c == '\r') {
		usb_putc('\n');
	}

   	switch(p_state) {
       case WAIT_FOR_NEWLINE:
           if(c == '\n' || c == '\r')
               p_state = WAIT_FOR_CMD_CHAR;
           break;
       case WAIT_FOR_CMD_CHAR:
           switch(c) {
               case 'o':
                   //loccStartOpening();
				   command = 1;
                   p_state = WAIT_FOR_NEWLINE;
                   break;
               case 'l':
                   usb_putc('L');
                   p_state = WAIT_FOR_LED_NUMBER;
                   break;
               case 'h':
                   //loccPowerDown();
                   break;
               case 'i':
                   //loccPowerUp();
                   break;
               case '\n':
                   usb_putc('\r');
                   break;
               default:
                   p_state = WAIT_FOR_NEWLINE;
				   break;
           }
           break;
       case WAIT_FOR_LED_NUMBER:
           cmd_target = c;
           p_state = WAIT_FOR_LED_VALUE;
           break;
       case WAIT_FOR_LED_VALUE:
           set_led(cmd_target, c-'0');
           p_state = WAIT_FOR_NEWLINE;
           break;
	}
	
	return command;
}

inline void usb_putc(char c){
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


/** Event handler for the CDC Class driver Line Encoding Changed event.
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
} 
 
/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}


