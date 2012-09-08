/*
 * sw.c
 *
 *  Created on: 16 jan 2012
 *      Author: Sterna
 */

#include "sw.h"

void initSW()
{
	//Set pins as inports (prog, start and stop)
	DDRD &= ~_BV(1) & ~_BV(2);
	DDRC &= ~_BV(0);

	//Set internal pullups
	PORTD |= _BV(1) | _BV(2);
	PORTC |= _BV(0);

	//Init dipswitch
	DDRC &=  ~0b00111110;
	//Set internal pullups
	PORTC |= 0b00111110;
}

/*
 * Returns the current value of the dip-switches
 * Please note that this does not directly correspond start/stop-command
 */
unsigned char readDip()
{
	return (~(PINC>>1) & 0b00011111);
}
