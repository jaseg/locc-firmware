/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/* //                                                                                                              // */
/* //                                                                                                              // */
/* //                                             myTSE Firmware                                                   // */
/* //                                                  V1.00                                                       // */
/* //                                                                                                              // */
/* //  Copyright (C) 2008 DXT                                                                                      // */
/* //                                                                                                              // */
/* //  This program is free software; you can redistribute it and/or modify                                        // */
/* //  it under the terms of the GNU General Public License as published by                                        // */
/* //  the Free Software Foundation; either version 2 of the License, or                                           // */
/* //  (at your option) any later version.                                                                         // */
/* //                                                                                                              // */
/* //  This program is distributed in the hope that it will be useful,                                             // */
/* //  but WITHOUT ANY WARRANTY; without even the implied warranty of                                              // */
/* //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                                               // */
/* //  GNU General Public License for more details.                                                                // */
/* //                                                                                                              // */
/* //  You should have received a copy of the GNU General Public License                                           // */
/* //  along with this program; if not, write to the Free Software                                                 // */
/* //  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA                                  // */
/* //                                                                                                              // */
/* //                                                                                                              // */
/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


/* // HEADER //////////////////////////////////////////////////////////////////////////////////////////////////////// */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include "locc.h"
#include "c_locc.h"

/* we are using TIMER0 here ! */
/* I/O Port configuration */
/* Note: The following code will not affect other I/O pins than the I/O Pins selected below. You can 
       	 use them freely for other proposes */

/* Cylinder IO */
#define CYLINDER_IO_DDR DDRD				/* Data direction register of the used port */
#define CYLINDER_IO_PORT PORTD				/* Port I/O Register of the used port */
#define CYLINDER_IO_PIN 5				/* Pin number of the used pin */

/* Cylinder 5V */
#define CYLINDER_VCC_DDR DDRD
#define CYLINDER_VCC_PORT PORTD				/* Port I/O Register of the used port */
#define CYLINDER_VCC_PIN 6				/* Pin number of the used pin */

#define AWAKETIME 59
#define CYLINDER_TELEGRAM_TIMING_DELAY 50


/* Opening datagram */
int statusTelegram[] =  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00};
int openTelegram[] =   {0x02, 0x23, 0x42, 0xb7, 0x00, 0x00, 0x63, 0x00};

enum locc_states { SLEEP, POWERUP, AWAKEN, OPEN, POWERDOWN, RESET };

volatile unsigned long wait_ticks = 0;
volatile enum locc_states state = SLEEP;

unsigned int current_locc_step = 0;

static void sendCylinderTelegram(int *telegram, int length);
static void state_machine(enum locc_states new_state);

void systemDelay10us(uint8_t value)
{
	while(value--)
		_delay_us(10);
	return;
}
void systemDelay1ms(uint8_t value)
{
	while(value--)
		_delay_ms(1);
	return;
}

void systemDelay10ms(uint8_t value)
{
	while(value--)
		_delay_ms(10);
	return;
}
void systemDelay1s(uint8_t value)
{
	while(value--)
		_delay_ms(1000);
	return;
}

/* Call this to start an opening process */
void loccStartOpening(void) {
    if(state == SLEEP) {
        usb_putc('|');
        state_machine(POWERUP);
    }
}

static void powerdown_locc(void) {
    /* don't let leak power over signal line to vcc on cylinder */
    CYLINDER_IO_DDR  |= (1 << CYLINDER_IO_PIN); /* Set I/O Pin to output (Low impedance) */
    CYLINDER_IO_PORT &= ~(1 << CYLINDER_IO_PIN);    /* Switch I/O pin to low */

    CYLINDER_VCC_PORT &= ~(1 << CYLINDER_VCC_PIN); 

}

static void powerup_locc(void) {
    CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN); /* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN); /* Switch internal pullup resistor on */
		CYLINDER_VCC_PORT |= (1 << CYLINDER_VCC_PIN);
}

static void wakeup_locc(void) {
    /* Wakeup Pulse */
    /* generate logical HIGH */
    CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN); /* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN); /* Switch internal pullup resistor on */
    systemDelay10us(CYLINDER_TELEGRAM_TIMING_DELAY);
    /* generate logical LOW */
    CYLINDER_IO_DDR  |= (1 << CYLINDER_IO_PIN); /* Set I/O Pin to output (Low impedance) */
    CYLINDER_IO_PORT &= ~(1 << CYLINDER_IO_PIN);    /* Switch I/O pin to low */
    systemDelay10us(2*CYLINDER_TELEGRAM_TIMING_DELAY);
    /* generate logical HIGH */
    CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN); /* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN); /* Switch internal pullup resistor on */
    // we will wait 59 ms now over timer/statemachine
}


static void open_locc(void) {
    sendCylinderTelegram(openTelegram, 8);
}

/* Send telegram to cylinder */
static void sendCylinderTelegram(int *telegram, int length)
{
	int i,j = 0;
	
	/* Telegram */
	for(j=0; j<=length;j++)
	{
		for(i=7; i>=0; i--)
			if((*telegram >> i) & 1)
			{
				/* generate logical HIGH */
				CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);	/* Set I/O Pin to input (High impedance) */
				CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);	/* Switch internal pullup resistor on */
				systemDelay10us(CYLINDER_TELEGRAM_TIMING_DELAY);
				/* generate logical LOW */
				CYLINDER_IO_DDR  |= (1 << CYLINDER_IO_PIN);	/* Set I/O Pin to output (Low impedance) */
				CYLINDER_IO_PORT &= ~(1 << CYLINDER_IO_PIN);	/* Switch I/O pin to low */
				systemDelay10us(CYLINDER_TELEGRAM_TIMING_DELAY);
			}
			else
			{
				/* generate logical LOW */
				CYLINDER_IO_DDR  |= (1 << CYLINDER_IO_PIN);	/* Set I/O Pin to output (Low impedance) */
				CYLINDER_IO_PORT &= ~(1 << CYLINDER_IO_PIN);	/* Switch I/O pin to low */
				systemDelay10us(CYLINDER_TELEGRAM_TIMING_DELAY);
				/* generate logical HIGH */
				CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);	/* Set I/O Pin to input (High impedance) */
				CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);	/* Switch internal pullup resistor on */
				systemDelay10us(CYLINDER_TELEGRAM_TIMING_DELAY);
			}

		telegram++;
	}

	/* generate logical HIGH */
	CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);				/* Set I/O Pin to input (High impedance) */
	CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);				/* Switch internal pullup resistor on */
}

static void state_machine(enum locc_states new_state) {
    state = new_state;
    switch(new_state) {
        case SLEEP:
            usb_putc('[');
            wait_ticks = 0;
            break;
        case POWERUP:
            usb_putc(']');
            powerup_locc();
            wait_ticks = 50;
            break;
        case AWAKEN:
            usb_putc('{');
            wakeup_locc();
            // we have to wait 59ms
            wait_ticks = 59;
            break;
        case OPEN:
            usb_putc('}');
            open_locc();
            // how long takes the locc to close cylinder?
            wait_ticks = 10000; /* 10 s */
            break;
            // the lock will automatic close the opener
        case POWERDOWN:
            usb_putc('"');
            powerdown_locc();
            wait_ticks = 2;
            break;
				case RESET:
						state_machine(SLEEP);
						break;
    }
}

void start_locking(void) {
	current_locc_step = 1;
}

bool do_next_locc_step(void) {
	
	// do nothing as long as the timer is still counting down to 0
	if (timer_is_expired()) {		
		do_locc_step(current_locc_step);
	} 
	
	// if we are back at step 0 then the lock process was completed
	if (current_locc_step > 0) {
		return true;
	} else {
		usb_putc('.');
		return false;
	}
	
}

static void do_locc_step(int step) {
	switch(current_locc_step) {
		// safe state for the beginning
		case 0:
			set_wait_time(0);
			current_locc_step = 1;
			break;
		
		// send the wake up call to the lock
		case 1:
			wakeup_locc();
			current_locc_step = 2;
			set_wait_time(59);
			break;
		
		// open the lock and leave it open for 10 s	
		case 2:
			open_locc();
			set_wait_time(15000);
			current_locc_step = 3;
			break;
			
		// switch of power for the lock (closes it and shuts it down)
		case 3:
			powerdown_locc();
			set_wait_time(5000);
			current_locc_step = 4;
			break;

		// power up the capacitor, 5 s should be enough for the capacitor to reach 5V.
		case 4:
			powerup_locc();
			set_wait_time(5000);
			current_locc_step = 0;
			break;
			
	}
}

/**
 * Return true if the timer is finished waiting the time that was set using 
 * set_wait_time() before.
 */
static bool timer_is_expired(void) {
	if (wait_ticks > 0) {
		return false;
	}
	return true;
}

/**
 * Set the timer's wait time. Timer is always running in the background and 
 * will automatically begin decrementing wait_ticks.
 */
static void set_wait_time(unsigned long millis) {
	wait_ticks = millis;
}

/* Configure I/O Ports */
void loccSetup(void)
{
	CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);	/* Setup cylinder I/O */
	CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);

	CYLINDER_VCC_DDR |= (1 << CYLINDER_VCC_PIN);		/* Setup cylinder VCC */
	CYLINDER_VCC_PORT &= ~(1 << CYLINDER_VCC_PIN);

    // Timer0 with prescaler 64 by 16mhz => 1msec
    TIMSK0 |= 1 << TOIE0;
    TCCR0B = 0x03;
    TCNT0 = 0x06;

	powerup_locc();
    //powerdown_locc();
}

/* doing state_machine stuff to handle code outside of ISR */
void loccPoll(void) {
		usb_putc('>');
    if((state != SLEEP) &&  (wait_ticks == 0)) {
        state_machine(++state);
    }
}

void loccPowerDown(void) {
    powerdown_locc();
}
void loccPowerUp(void) {
    powerup_locc();
}

/* proxy method to state_tick */
ISR(TIMER0_OVF_vect) {
    TCNT0 = 0x06;
    // if((state != SLEEP) &&  (wait_ticks > 0)) {
	if (wait_ticks > 0) {
    	wait_ticks--;
    }
}
