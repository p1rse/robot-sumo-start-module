/*
 * sw.c
 *
 *  Created on: 16 jan 2012
 *      Author: Sterna
 */


#include <avr/io.h>
#include "led.h"

/*
 * Initialize the LEDs
 */
void initLed()
{
	// Set LED bits [0-3] on Port B as output
	DDRB |= 0b00001111;
}

/*
 *  Sets LED to desired state
 *  The value led may be 1-4 and state may be ON or OFF (1 or 0)
 */
void setLed(unsigned char led,unsigned char state)
{
	if(led>0 && led<5)
	{
		if(state == OFF)
		{
			PORTB &= ~(0b00000001 << (led-1));
		}
		else
		{
			PORTB |= (0b00000001 << (led-1));
		}
	}
}

/*
 * Sets all LED to desired state
 * state may be ON or OFF (1 or 0)
 */
void setAllLeds(unsigned char state)
{
	if(state == OFF)
	{
		PORTB &= 0b11110000;
	}
	else
	{
		PORTB |= 0b00001111;
	}
}
