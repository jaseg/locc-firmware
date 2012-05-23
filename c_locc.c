/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/* //                                                                                                              // */
/* //                                                                                                              // */
/* //                                             myTSE Firmware                                                   // */
/* //                                                  V1.23                                                       // */
/* //                                                                                                              // */
/* //  Copyright (C) 2008 DXT, 2011 jaseg                                                                          // */
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

#include <avr/io.h>
#include <util/delay.h>
#include "c_locc.h"

#define CYLINDER_IO_DDR DDRD				/* Data direction register of the used port */
#define CYLINDER_IO_PORT PORTD				/* Port I/O Register of the used port */
#define CYLINDER_IO_PIN 5				/* Pin number of the used pin */

#define CYLINDER_POWER_DDR DDRD
#define CYLINDER_POWER_PORT PORTD
#define CYLINDER_POWER_PIN 6

uint8_t wakeup_datagram[] =  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00};
#include "secret.h"

#define CYLINDER_TELEGRAM_TIMING_DELAY 50

void c_locc_cetup(void){
    CYLINDER_IO_DDR &= ~_BV(CYLINDER_IO_PIN);	/* Setup cylinder I/O */
    CYLINDER_IO_PORT |= _BV(CYLINDER_IO_PIN);
    CYLINDER_POWER_DDR |= _BV(CYLINDER_POWER_PIN);
}

void _delay_us_x10(uint8_t delay){
    for(uint8_t i=0; i<delay; i++){
        _delay_us(10);
    }
}

/* Send telegram to cylinder */
void sendCylinderTelegram(uint8_t *telegram, uint8_t length)
{
    /* Wakeup Pulse */
    /* generate logical HIGH */
    CYLINDER_IO_DDR &= ~_BV(CYLINDER_IO_PIN); /* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= _BV(CYLINDER_IO_PIN); /* Switch internal pullup resistor on */
    _delay_us_x10(CYLINDER_TELEGRAM_TIMING_DELAY);
    /* generate logical LOW */
    CYLINDER_IO_DDR  |= _BV(CYLINDER_IO_PIN); /* Set I/O Pin to output (Low impedance) */
    CYLINDER_IO_PORT &= ~_BV(CYLINDER_IO_PIN);    /* Switch I/O pin to low */
    _delay_us_x10(2*CYLINDER_TELEGRAM_TIMING_DELAY);
    /* generate logical HIGH */
    CYLINDER_IO_DDR &= ~_BV(CYLINDER_IO_PIN); /* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= _BV(CYLINDER_IO_PIN); /* Switch internal pullup resistor on */
    _delay_ms(59);

    /* Telegram */
    for(uint8_t j=0; j<=length;j++){
        for(uint8_t i=7; i>=0; i--){
            if((*telegram >> i) & 1)
            {
                /* generate logical HIGH */
                CYLINDER_IO_DDR &= ~_BV(CYLINDER_IO_PIN);	/* Set I/O Pin to input (High impedance) */
                CYLINDER_IO_PORT |= _BV(CYLINDER_IO_PIN);	/* Switch internal pullup resistor on */
                _delay_us_x10(CYLINDER_TELEGRAM_TIMING_DELAY);
                /* generate logical LOW */
                CYLINDER_IO_DDR  |= _BV(CYLINDER_IO_PIN);	/* Set I/O Pin to output (Low impedance) */
                CYLINDER_IO_PORT &= ~_BV(CYLINDER_IO_PIN);	/* Switch I/O pin to low */
                _delay_us_x10(CYLINDER_TELEGRAM_TIMING_DELAY);
            }else{
                /* generate logical LOW */
                CYLINDER_IO_DDR  |= _BV(CYLINDER_IO_PIN);	/* Set I/O Pin to output (Low impedance) */
                CYLINDER_IO_PORT &= ~_BV(CYLINDER_IO_PIN);	/* Switch I/O pin to low */
                _delay_us_x10(CYLINDER_TELEGRAM_TIMING_DELAY);
                /* generate logical HIGH */
                CYLINDER_IO_DDR &= ~_BV(CYLINDER_IO_PIN);	/* Set I/O Pin to input (High impedance) */
                CYLINDER_IO_PORT |= _BV(CYLINDER_IO_PIN);	/* Switch internal pullup resistor on */
                _delay_us_x10(CYLINDER_TELEGRAM_TIMING_DELAY);
            }
        }
        telegram++;
    }

    /* generate logical HIGH */
    CYLINDER_IO_DDR &= ~_BV(CYLINDER_IO_PIN);				/* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= _BV(CYLINDER_IO_PIN);				/* Switch internal pullup resistor on */
}

//Please call c_locc_deactivate after 9s later
void c_locc_open(void){
    CYLINDER_POWER_PORT |= _BV(CYLINDER_POWER_PIN);
    //sendCylinderTelegram(wakeup_datagram,8);
    //_delay_ms(10);
    sendCylinderTelegram(open_datagram,8);
}

//Needs to be called at least 9s after c_locc_open
void c_locc_deactivate(void){
    CYLINDER_POWER_PORT &= ~_BV(CYLINDER_POWER_PIN);
}
