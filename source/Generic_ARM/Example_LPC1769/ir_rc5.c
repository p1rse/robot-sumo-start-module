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
 */

#include "ir.h"

#ifdef	IR_RC5_PROTOCOL
#include "LPC17xx.h"

/*
 * Protocol definitions
 */
#define ADDR_LEN	5
#define CMD_LEN		6

/*
 * Timings in us
 */
#define HALF_BIT_LEN	889UL
#define BIT_LEN			1778UL

/*
 * Timer constants
 */
#define MAX_ERROR		((IR_COUNTER_CLOCK * 24UL) / 100000UL)	// Maximum timing error in timer ticks

#define HALF_BIT_CNT	(HALF_BIT_LEN * (IR_COUNTER_CLOCK / 1000000L))
#define BIT_CNT			(BIT_LEN * (IR_COUNTER_CLOCK / 1000000L))

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
volatile static int8_t status;
volatile static int8_t toggle;
volatile static uint8_t repeats;
volatile static uint8_t buffer_read, buffer_write;

/*
 * Buffer with received data
 */
volatile static IRDATA buffer[IR_BUFFER_SIZE];

/*
 * Data handler function
 */
static void(*ir_data_func)(IRDATA *data) = 0;

/*
 * Debug
 */
#if IR_DBG_EN
volatile unsigned int 			cnt_start2,
								cnt_toggle,
								cnt_addr,
								cnt_cmd;
#endif


void ir_init(void) {
	/*
	 * Variables
	 */
	status = IR_STARTBIT1;
	buffer_read = 0;
	buffer_write = 0;
	toggle = 0;

#if IR_DBG_EN
	cnt_start2 = 0;
	cnt_toggle = 0;
	cnt_addr = 0;
	cnt_cmd = 0;
#endif

	IR_INT_INIT();
	IR_TIMER_INIT();

	IR_FALLING_INT();
}

void ir_set_data_handler(void(*func)(IRDATA *data)) {
	ir_data_func = func;
}

void ir_remove_data_handler() {
	ir_data_func = 0;
}

signed char ir_has_next(void) {
	if (buffer_read == buffer_write) {
		return 0;
	} else {
		return 1;
	}
}

void ir_get_next(IRDATA *data) {

	while(buffer_read == buffer_write);

	data->address_low = buffer[buffer_read].address_low;
	data->command = buffer[buffer_read].command;

	if (buffer_read < IR_BUFFER_SIZE - 1) {
		buffer_read++;
	} else {
		buffer_read = 0;
	}
}

unsigned char ir_get_repeats(void) {
	return repeats;
}

IR_INT_ISR {
	IR_CLR_INT_ISR();

	static uint8_t index = 0, addr = 0, cmd = 0, toggle_last = 0;

	// Workaround to interrupt on any edge
	if (IR_EDGE_LOW()) {
		IR_RISING_INT();
	} else {
		IR_FALLING_INT();
	}

	switch (status) {
	case IR_STARTBIT1:
		if (IR_EDGE_LOW()) {
			status = IR_STARTBIT2;
			IR_CNT = 0;
		} else {
			status = IR_STARTBIT1;
		}
		break;

	case IR_STARTBIT2:

#if IR_DBG_EN
		cnt_start2 = IR_CNT;
#endif

		if ((IR_CNT - MAX_ERROR) < BIT_CNT && (IR_CNT + MAX_ERROR) > BIT_CNT) {
			status = IR_TOGGLEBIT;
			IR_CNT = 0;

		} else if ((IR_CNT - MAX_ERROR) < HALF_BIT_CNT && (IR_CNT + MAX_ERROR) > HALF_BIT_CNT) {
			IR_CNT = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;

	case IR_TOGGLEBIT:

#if IR_DBG_EN
		cnt_toggle = IR_CNT;
#endif

		if ((IR_CNT - MAX_ERROR) < BIT_CNT && (IR_CNT + MAX_ERROR) > BIT_CNT) {

			toggle_last = toggle;

			if (IR_EDGE_LOW()) {
				toggle = 1;
			} else {
				toggle = 0;
			}

			index = 0;
			addr = 0;
			cmd = 0;

			IR_CNT = 0;
			status = IR_ADDR;

		} else if ((IR_CNT - MAX_ERROR) < HALF_BIT_CNT && (IR_CNT + MAX_ERROR) > HALF_BIT_CNT) {
			IR_CNT = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;

	case IR_ADDR:

#if IR_DBG_EN
		cnt_addr = IR_CNT;
#endif

		if ((IR_CNT - MAX_ERROR) < BIT_CNT && (IR_CNT + MAX_ERROR) > BIT_CNT) {

			if (IR_EDGE_LOW()) {
				addr |= _BV(ADDR_LEN - index - 1);
			} else {

			}

			index++;
			if (index == ADDR_LEN) {
				index = 0;
				status = IR_CMD;
			}
			IR_CNT = 0;
		} else if ((IR_CNT - MAX_ERROR) < HALF_BIT_CNT && (IR_CNT + MAX_ERROR) > HALF_BIT_CNT) {
			IR_CNT = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;

	case IR_CMD:
		if ((IR_CNT - MAX_ERROR) < BIT_CNT && (IR_CNT + MAX_ERROR) > BIT_CNT) {

#if IR_DBG_EN
		cnt_cmd = IR_CNT;
#endif

			if (IR_EDGE_LOW()) {
				cmd |= _BV(CMD_LEN - index - 1);
			} else {

			}

			index++;
			if (index == CMD_LEN) {
				if (toggle == toggle_last && !IR_RC5_DISABLE_REPEATS) {
					repeats++;
				} else {
					if (ir_data_func) {
						static IRDATA tmp_data;
						tmp_data.address_low = addr;
						tmp_data.command = cmd;
						ir_data_func(&tmp_data);
					} else {
						buffer[buffer_write].address_low = addr;
						buffer[buffer_write].command = cmd;

						if (buffer_write < IR_BUFFER_SIZE - 1) {
							buffer_write++;
						} else {
							buffer_write = 0;
						}
					}

					repeats = 0;
				}

				status = IR_STARTBIT1;
			}

			IR_CNT = 0;
		} else if ((IR_CNT - MAX_ERROR) < HALF_BIT_CNT && (IR_CNT + MAX_ERROR) > HALF_BIT_CNT) {
			IR_CNT = HALF_BIT_CNT;	// Synchronize..
			return;
		} else {
			status = IR_STARTBIT1;
			return;
		}
		break;
	}
}

IR_CNT_ISR {
	IR_CLR_TIM_ISR();

	status = IR_STARTBIT1;
}

#endif


