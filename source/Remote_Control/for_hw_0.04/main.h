/*
 * main.h
 *
 *  Created on: 17 jan 2012
 *      Author: Sterna
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>

#include "rc5.h"
#include "led.h"
#include "sw.h"

#define PROG 0
#define START 1
#define STOP 2

//Should be about 2.4 V
#define LOW_VOLTAGE 115


void sendProgCmd();
void sendStart(unsigned char repeats);
void sendStop(unsigned char repeats);
void goToSleep();

#endif /* MAIN_H_ */
