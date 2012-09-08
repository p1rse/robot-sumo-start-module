/*
 * main.c
 *
 *  Created on: 15 jan 2012
 *      Author: benjamin
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "LPC17xx.h"
#include "start.h"

/*
 * Start functions
 */
void start_save_data(START_DATA *data);
void start_load_data(START_DATA *data);
void start_state_changed(char new_state);
void start_programming_done(char new_stop_cmd);

/*
 * Macros
 */
#define	_BV(bit) 						(1<<(bit))
#define LED_ON()						(LPC_GPIO0->FIOSET = _BV(22))
#define LED_OFF()						(LPC_GPIO0->FIOCLR = _BV(22))
#define LED_IS_ON						(LPC_GPIO0->FIOPIN & _BV(22))

void SysTick_Handler(void) {
	static int cnt = 0;
	cnt++;
	if (cnt >= 10) {
		cnt = 0;
		start_timerfunc();
	}
}

void init() {
	SystemCoreClockUpdate();

	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1);
	}

	NVIC_SetPriority (SysTick_IRQn, 2);

	// LED Pin
	LPC_GPIO0->FIODIR |= _BV(22);

	// IR Pins
	LPC_GPIO2->FIODIR |= _BV(7) | _BV(8);
	LPC_GPIO2->FIOSET = _BV(7);
	LPC_GPIO2->FIOCLR = _BV(8);

	// Start module
	start_init(start_save_data, start_load_data,
				start_programming_done, start_state_changed);
}

int main (void) {
	init();

	for(;;) {

	}
}

/*
 * Delay without the system timer. Works in interrupts.
 */
void delay_no_timer() {
	volatile uint32_t tmp = 0;
	for (uint32_t i = 0;i < SystemCoreClock / 120;i++) {
		for (uint32_t j = 0;j < 10;j++) {
			tmp++;
		}
	}
}

void start_save_data(START_DATA *data) {
	/*
	 * Save start data to non-volatile memory. LPC1768/1769 has
	 * no eeprom, so you have to find some way to store it.
	 */
}

void start_load_data(START_DATA *data) {
	/*
	 * Read start data from non-volatile memory. You have to implement this
	 * yourself.
	 */
}

void start_state_changed(char new_state) {
	/*
	 * State has changed. Indicated using an LED in this example.
	 */

	if (LED_IS_ON) {
		LED_OFF();
	} else {
		LED_ON();
	}

	if (new_state == START_STATE_STOPPED) {
		for(;;) {
			LED_ON();
			delay_no_timer();
			LED_OFF();
			delay_no_timer();
		}
	}
}

void start_programming_done(char new_stop_cmd) {
	/*
	 * Indicate that programming is done.
	 * Example using an LED.
	 */

	LED_OFF();
	delay_no_timer();
	LED_ON();
	delay_no_timer();
	LED_OFF();
	delay_no_timer();
	LED_ON();
	delay_no_timer();
	LED_OFF();
}
