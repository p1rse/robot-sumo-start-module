/*
 * rc5.c
 *
 *  Created on: 16 jan 2012
 *      Author: benjamin and Sterna
 */

#include "rc5.h"

void initIR(void)
{

	/*
	 * Progled
	 * Timer2
	 * CTC
	 * 38kHz @ 8MHz F_CPU
	 */
	DDRD |= _BV(3);
	TCCR2A |= _BV(WGM21);
	TCCR2B |= _BV(CS20);
	OCR2A = 105;

	/*
	 * CMDleds
	 * Timer0
	 * CTC
	 * 38kHz @ 8MHz F_CPU
	 */
	DDRD |= _BV(5);
	TCCR0A |= _BV(WGM01);
	TCCR0B |= _BV(CS00);
	OCR0A = 105;
}

/*
 * IR diode on (Pulse)
 * Toggle OC2B on compare match.
 */
void ir_on(unsigned char led)
{
	if (led == CMDLED)
	{
		TCCR0A |= _BV(COM0B0);
	}
	else if (led == PROGLED)
	{
		TCCR2A |= _BV(COM2B0);
	}
}

void ir_off(unsigned char led)
{
	if (led == CMDLED)
	{
		TCCR0A &= ~_BV(COM0B0);
		PORTD &= ~_BV(5);
	}
	else if (led == PROGLED)
	{
		TCCR2A &= ~_BV(COM2B0);
		PORTD &= ~_BV(3);
	}
}

void send_one(unsigned char led)
{
	ir_off(led);
	_delay_us(889);
	ir_on(led);
	_delay_us(889);
}

void send_zero(unsigned char led)
{
	ir_on(led);
	_delay_us(889);
	ir_off(led);
	_delay_us(889);
}

/*
 * Sends an RC-5 packet, with address "addr", command "cmd" from the led "led"
 * led can be CMDLED (front leds) or PROGLED (back led)
 */
void send_packet(unsigned char addr, unsigned char cmd, unsigned char led)
{
	static unsigned char toggle_last = 0;

	send_one(led);
	send_one(led);

	if (toggle_last) {
		toggle_last = 0;
		send_one(led);
	} else {
		toggle_last = 1;
		send_zero(led);
	}

	unsigned char i;
	for (i = 0; i < 5; i++) {
		if (addr & _BV(4 - i)) {
			send_one(led);
		} else {
			send_zero(led);
		}
	}

	for (i = 0; i < 6; i++) {
		if (cmd & _BV(5 - i)) {
			send_one(led);
		} else {
			send_zero(led);
		}
	}

	ir_off(led);
}
