/*
 * main.c
 *
 *  Created on: 18 nov 2011
 *      Author: Sterna, Benjamin
 *
 *  NOTE:
 *  This code looks like crap, but it had to be optimized to save
 *  enough bytes to fit on this processor.
 *
 */


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "ir.h"

/*
 * EEPROM addresses
 */
#define E_STATE_ADDRESS		8
#define E_STOPCMD_ADDRESS	10

/*
 * Pins
 */
#define LEDPIN		3
#define STARTPIN	2
#define KILLSWPIN	1
#define MODEPIN		0

/*
 * States
 */
#define POWERON			1
#define STARTED			2
#define STOPPED_SAFE	3
#define STOPPED			4

/*
 * Home mode commands
 */
#define HOME_START	RC5_CMD_1
#define HOME_STOP	RC5_CMD_2

/*
 * Default stop command (used after programming when eeprom is cleared)
 */
#define DEFAULT_STOP_CMD	0x04

/*
 * IO Macros
 */
#define LED_ON()		(PORTB |= _BV(LEDPIN))
#define LED_OFF()		(PORTB &= ~_BV(LEDPIN))
#define IS_COMP_MODE	(PINB & _BV(0))
//#define IS_COMP_MODE	(1) // Permanent competition mode

/*
 * Calling this function requires less memory than using _delay_ms
 * in several places.
 */
void delay_500() {
	_delay_ms(500);
}

// Using a function and function calls takes less memory..
void eeprom_write_byte_cli(unsigned char address, unsigned char state) {
	cli();
	eeprom_write_byte((unsigned char*)(unsigned int)address, state);
	sei();
}

int main() {
	ir_init();

	unsigned char currentState = eeprom_read_byte((unsigned char*)E_STATE_ADDRESS);
	unsigned char stopCommand = eeprom_read_byte((unsigned char*)E_STOPCMD_ADDRESS);
	unsigned char savedState = currentState;

	DDRB = _BV(LEDPIN) | _BV(KILLSWPIN) | _BV(STARTPIN);
	PORTB = _BV(MODEPIN); // Pull-up on mode select pin

	sei();

	for(;;) {
		while(!has_next) {
			switch (currentState) {
			case POWERON:
				PORTB = _BV(MODEPIN) | _BV(KILLSWPIN);
				break;

			case STARTED:
				PORTB = _BV(STARTPIN) | _BV(LEDPIN) | _BV(KILLSWPIN) | _BV(MODEPIN);
				break;

			case STOPPED_SAFE:
				PORTB = _BV(MODEPIN);
				eeprom_write_byte_cli(E_STATE_ADDRESS, STOPPED_SAFE);
				delay_500();
				delay_500();
				currentState = STOPPED;
				has_next = 0;
				break;

			case STOPPED:
				PORTB = _BV(MODEPIN);
				eeprom_write_byte_cli(E_STATE_ADDRESS, POWERON);
				for (;;) {
					LED_ON();
					delay_500();
					delay_500();
					LED_OFF();
					delay_500();
					delay_500();
				}
				break;

				// When eeprom is undefined we might end up here (after programming)
			default:
				currentState = STOPPED;
				stopCommand = DEFAULT_STOP_CMD;
				break;
			}

			if (savedState != currentState && currentState != STOPPED) {
				eeprom_write_byte_cli(E_STATE_ADDRESS, currentState);
				delay_500();
				savedState = currentState;
			}
		}
		has_next = 0;

		if (addr == RC5_ADR_PROGRAMMING) {
			stopCommand = cmd & 0b11111110;
			LED_ON();
			delay_500();
			eeprom_write_byte_cli(E_STOPCMD_ADDRESS, stopCommand);
			LED_OFF();
			delay_500();
			LED_ON();
			delay_500();
			LED_OFF();
			has_next = 0;
			currentState = POWERON;
		} else {
			switch (currentState) {
				case POWERON:
				if (IS_COMP_MODE) {
					if (addr == RC5_ADR_EXPERIMENTAL) {
						if (cmd == (stopCommand + 1)) {
							currentState = STARTED;
						} else if (cmd == stopCommand) {
							currentState = STOPPED_SAFE;
						}
					}
				} else {
					if (cmd == HOME_START) {
						currentState = STARTED;
					} else if (cmd == HOME_STOP) {
						currentState = STOPPED_SAFE;
					}
				}
				break;

			case STARTED:
				if (IS_COMP_MODE) {
					if (addr == RC5_ADR_EXPERIMENTAL) {
						if (cmd == stopCommand) {
							currentState = STOPPED_SAFE;
						}
					}
				} else {
					if (cmd == HOME_STOP) {
						currentState = STOPPED_SAFE;
					}
				}
				break;
			}
		}
	}

	return(0);
}
