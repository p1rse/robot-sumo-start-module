/*
 * main.c
 *
 *  Created on: 20 jan 2012
 *      Author: benjamin
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "ir.h"
#include "start.h"

/*
 * Settings
 */
#define EEPROM_START_DATA_LOCATION	42

/*
 * Functions
 */
void start_save_data(START_DATA *data);
void start_load_data(START_DATA *data);
void start_state_changed(char new_state);
void start_programming_done(char new_stop_cmd);

ISR (TIMER1_COMPA_vect) {
	start_timerfunc();
}

void start_save_data(START_DATA *data) {
	eeprom_write_block((const void*)data,
			(void*)EEPROM_START_DATA_LOCATION, sizeof(START_DATA));
}

void start_load_data(START_DATA *data) {
	eeprom_read_block((void*)data,
			(const void*)EEPROM_START_DATA_LOCATION, sizeof(START_DATA));
}

/*
 * Simple versions if you don't like structs...
 */
//void start_save_data_simple(START_DATA *data) {
//	char state = data->state;
//	char cmd = data->stop_cmd;
//
//	// Save to eeprom here...
//}
//
//void start_load_data_simple(START_DATA *data) {
//	char state;
//	char cmd;
//
//	// Load from eeprom here...
//
//	data->state = state;
//	data->stop_cmd = cmd;
//}

void start_state_changed(char new_state) {
	// TODO: Start or stop your robot, this could also be done in you main loop
	// You should also indicate with LED or display the different states, Normally:
	//  START_STATE_POWER_ON: LED OFF
	//	START_STATE_STARTED: LED ON
	//	START_STATE_STOPPED_SAFE & START_STATE_STOPPED: LED Flashing
}

void start_programming_done(char new_stop_cmd) {
	// TODO: Flash twice with LED or indicate on display
}

int main() {
	clock_prescale_set(0);

	// Initialize start module
	start_init(start_save_data, start_load_data,
			start_programming_done, start_state_changed);

	// Initialize timer to generate interrupts at 100Hz
	TCCR1B |= _BV(CS11);	// Prescale 8
	TCCR1B |= _BV(WGM12);	// CTC.
	OCR1A = 10000;	// 100 Hz
	TIMSK1 |= _BV(OCIE1A);

	// Enable interrupts
	sei();

	for(;;) {
		if (start_get_state() == START_STATE_STARTED ){
			//Start your robot, or do it in the start_state_changed above, this is completely depending on how you decide to implement it.
		}

	}

	return 0;
}
