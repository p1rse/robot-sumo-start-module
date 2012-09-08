/*
	Copyright 2009-2011 Benjamin Vedder	vedder87@gmail.com

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

/*
 * ir_rc5.c
 *
 *  Created on: 26 dec 2009
 *      Author: benjamin
 *
 * Note: This is a minimal version of the code to run on
 * systems with limited program memory. It is recommended
 * to use the full version if possible.
 */

#include "ir.h"

#include <avr/io.h>
#include <avr/interrupt.h>


/*
 * Protocol definitions
 */
#define ADDR_LEN	5
#define CMD_LEN		6

/*
 * Timings in us
 */
#define HALF_BIT_LEN	889L
#define BIT_LEN			1778L

/*
 * Timer constants
 */
#define MAX_ERROR		((F_CPU * 5) / 1000000)	// Maximum timing error in timer ticks
#define TIMER_PRESCALE	64

#define HALF_BIT_CNT	((HALF_BIT_LEN * (F_CPU / 1000000L)) / TIMER_PRESCALE)	//111.12 @8MHz
#define BIT_CNT			((BIT_LEN * (F_CPU / 1000000L)) / TIMER_PRESCALE)		//222.25 @8MHz

/*
 * Interrupt stuff
 */
#define	IR_STARTBIT1	0
#define	IR_STARTBIT2	1
#define	IR_TOGGLEBIT	2
#define	IR_ADDR			3
#define	IR_CMD			4

/*
 * Variables
 */
volatile signed char status;
volatile unsigned char has_next;
volatile unsigned char timer_overflows;

/*
 * Received data
 */
volatile unsigned char cmd;
volatile unsigned char addr;

/*
 * Interrupt data
 */
register unsigned char curr_index asm("r2");


void ir_init(void) {
	/*
	 * Variables
	 */
	status = IR_STARTBIT1;
	has_next = 0;

	curr_index = 0;

	/*
	 * Init Timer0
	 */
	IR_CNT_PRESCALE_SET();
	IR_CNT = 0;
	IR_CNT_OVF_EN();

	/*
	 * Init INT0
	 */
	IR_INT_EN();		// Enable int0
	IR_ANY_EDGE_INT();	// Interrupt on any logical change
}

//signed char ir_has_next(void) {
//	return has_next;
//}

//void ir_get_next(unsigned char *addr_ptr, unsigned char *cmd_ptr) {
//	while(!has_next){};
//
//	*addr_ptr = addr;
//	*cmd_ptr = cmd;
//	has_next = 0;
//}

ISR (IR_PIN_ISR) {
	static signed int cnt = 0;

	cnt += (signed int)IR_CNT + (signed int)(((unsigned int)timer_overflows) << 8);	// Save count
	IR_CNT = 0;
	timer_overflows = 0;

	switch (status) {
	case IR_STARTBIT1:
		if (IR_EDGE_LOW()) {
			status = IR_STARTBIT2;
			cnt = 0;
		} else {
			status = IR_STARTBIT1;
		}
		break;

	case IR_STARTBIT2:
		if ((cnt - MAX_ERROR) < BIT_CNT && (cnt + MAX_ERROR) > BIT_CNT) {
			status = IR_TOGGLEBIT;
			cnt = 0;
		} else if ((cnt - MAX_ERROR) < HALF_BIT_CNT && (cnt + MAX_ERROR) > HALF_BIT_CNT) {
			cnt = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;

	case IR_TOGGLEBIT:
		if ((cnt - MAX_ERROR) < BIT_CNT && (cnt + MAX_ERROR) > BIT_CNT) {
			curr_index = ADDR_LEN;
			addr = 0;
			cmd = 0;
			cnt = 0;
			status = IR_ADDR;
		} else if ((cnt - MAX_ERROR) < HALF_BIT_CNT && (cnt + MAX_ERROR) > HALF_BIT_CNT) {
			cnt = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;

	case IR_ADDR:
		if ((cnt - MAX_ERROR) < BIT_CNT && (cnt + MAX_ERROR) > BIT_CNT) {
			curr_index--;
			if (IR_EDGE_LOW()) {
				addr |= _BV(curr_index);
			}

			if (!curr_index) {
				curr_index = CMD_LEN;
				status = IR_CMD;
			}
			cnt = 0;
		} else if ((cnt - MAX_ERROR) < HALF_BIT_CNT && (cnt + MAX_ERROR) > HALF_BIT_CNT) {
			cnt = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}

		break;

	case IR_CMD:
		if ((cnt - MAX_ERROR) < BIT_CNT && (cnt + MAX_ERROR) > BIT_CNT) {
			curr_index--;
			if (IR_EDGE_LOW()) {
				cmd |= _BV(curr_index);
			}

			if (!curr_index) {
				has_next = 1;
				status = IR_STARTBIT1;
			}
			cnt = 0;
		} else if ((cnt - MAX_ERROR) < HALF_BIT_CNT && (cnt + MAX_ERROR) > HALF_BIT_CNT) {
			cnt = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;
	}
}

ISR (IR_CNT_ISR) {
	timer_overflows++;
	if (timer_overflows > 50) {
		status = IR_STARTBIT1;
	}

}



