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
#include <util/delay.h>

/* I/O Port configuration */
/* Note: The following code will not affect other I/O pins than the I/O Pins selected below. You can 
   use them freely for other proposes */

/* Cylinder */
#define CYLINDER_IO_DDR DDRD				/* Data direction register of the used port */
#define CYLINDER_IO_PORT PORTD				/* Port I/O Register of the used port */
#define CYLINDER_IO_PIN 5				/* Pin number of the used pin */

/* Opening datagram */
/* Note: This is the opening datagram that will be sent to the cylinder this example contains the default
   code (123456) which is the factory setup */
uint8_t wakeupTelegram[] =  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00};
uint8_t openTelegram[]   = 	{0x02, 0x12, 0x34, 0x56, 0x00, 0x00, 0x5c, 0x00};
#define CYLINDER_TELEGRAM_TIMING_DELAY 400

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


/* // IMPLEMENTATION //////////////////////////////////////////////////////////////////////////////////////////////// */

/* Configure I/O Ports */
void c_locc_cetup(void){
    CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);	/* Setup cylinder I/O */
    CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);
}

/* Send telegram to cylinder */
void sendCylinderTelegram(uint8_t *telegram, uint8_t length)
{
    /* Telegram */
    for(uint8_t j=0; j<=length;j++){
        for(uint8_t i=7; i>=0; i--){
            if((*telegram >> i) & 1)
            {
                /* generate logical HIGH */
                CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);	/* Set I/O Pin to input (High impedance) */
                CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);	/* Switch internal pullup resistor on */
                _delay_us(CYLINDER_TELEGRAM_TIMING_DELAY);
                /* generate logical LOW */
                CYLINDER_IO_DDR  |= (1 << CYLINDER_IO_PIN);	/* Set I/O Pin to output (Low impedance) */
                CYLINDER_IO_PORT &= ~(1 << CYLINDER_IO_PIN);	/* Switch I/O pin to low */
                _delay_us(CYLINDER_TELEGRAM_TIMING_DELAY);
            }else{
                /* generate logical LOW */
                CYLINDER_IO_DDR  |= (1 << CYLINDER_IO_PIN);	/* Set I/O Pin to output (Low impedance) */
                CYLINDER_IO_PORT &= ~(1 << CYLINDER_IO_PIN);	/* Switch I/O pin to low */
                _delay_us(CYLINDER_TELEGRAM_TIMING_DELAY);
                /* generate logical HIGH */
                CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);	/* Set I/O Pin to input (High impedance) */
                CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);	/* Switch internal pullup resistor on */
                _delay_us(CYLINDER_TELEGRAM_TIMING_DELAY);
            }
        }
        telegram++;
    }

    /* generate logical HIGH */
    CYLINDER_IO_DDR &= ~(1 << CYLINDER_IO_PIN);				/* Set I/O Pin to input (High impedance) */
    CYLINDER_IO_PORT |= (1 << CYLINDER_IO_PIN);				/* Switch internal pullup resistor on */
}

void c_locc_open(void){
    sendCylinderTelegram(wakeupTelegram,8);
    _delay_ms(10);
    sendCylinderTelegram(openTelegram,8);
}
