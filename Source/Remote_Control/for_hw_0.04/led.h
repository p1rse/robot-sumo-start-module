/*
 * sw.h
 *
 *  Created on: 16 jan 2012
 *      Author: Sterna
 */

#ifndef LED_H
#define LED_H

#define OFF 0
#define ON 1


// Initialize the LEDs 
void initLed();

// Sets LED to desired state
void setLed(unsigned char led,unsigned char state);
void setAllLeds(unsigned char state);

#endif //LED_H

