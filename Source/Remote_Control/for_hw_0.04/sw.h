/*
 * sw.h
 *
 *  Created on: 16 jan 2012
 *      Author: Sterna
 */

#ifndef SW_H_
#define SW_H_

#include <avr/io.h>

/*
 * I use internal pullups on all switches, thus receiving logical 0 at button press
 */

#define SW_PROG (!(PIND & _BV(1)))
#define SW_STOP (!(PIND & _BV(2)))
#define SW_START (!(PINC & _BV(0)))
#define SW_DIP (~(PINC>>1) & 0b00011111)

void initSW();

unsigned char readDip();

#endif /* SW_H_ */
