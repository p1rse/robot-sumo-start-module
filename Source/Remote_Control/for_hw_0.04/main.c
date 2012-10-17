/*
 * main.c
 *
 *  Created on: 13 aug 2010
 *      Author: benjamin and sterna
 */

#include "main.h"

volatile unsigned char buttonPressed=0;

int main(void)
{
	//Init stuff
	clock_prescale_set(0);
	initIR();
	initSW();
	initLed();

	while(1)
	{
		if (buttonPressed == STOP)
		{
			//debounce
			while(SW_STOP)
			{
				_delay_ms(10);
				while(SW_STOP)	//Commands will be sent as long as button is held
				{
					sendStop(1);
				}
				//wait until button is released, to avoid sending unintentional commands
				while(SW_STOP){}
				_delay_ms(10);
			}
			_delay_ms(100);
		}
		else if (buttonPressed == START)
		{
			//debounce
			while(SW_START)
			{
				_delay_ms(10);
				while(SW_START)	//Commands will be sent as long as button is held
				{
					sendStart(1);
				}
				//wait until button is released, to avoid sending unintentional commands
				while(SW_START){}
				_delay_ms(10);
			}
			_delay_ms(100);
		}
		else if(buttonPressed == PROG)
		{
			//debounce
			while(SW_PROG)
			{
				_delay_ms(10);
				if (SW_PROG)
				{
					sendProgCmd();
				}
				//wait until button is released, to avoid sending unintentional commands
				while(SW_PROG){}
				_delay_ms(10);
			}
			_delay_ms(200);
		}
		goToSleep();
	}


	//Used for testing the control (does not rely on the power-down functions)
	while(1)
	{
		if (SW_STOP)
		{
			//debounce
			_delay_ms(10);
			if(SW_STOP)
			{
				sendStop(20);
			}
			//wait until button is released, to avoid sending unintentional commands
			while(SW_STOP){}
			_delay_ms(200);
		}
		else if (SW_START)
		{
			//debounce
			_delay_ms(10);
			if (SW_START)
			{
				sendStart(20);
			}
			//wait until button is released, to avoid sending unintentional commands
			while(SW_START){}
			_delay_ms(200);
		}
		else if(SW_PROG)
		{
			//debounce
			_delay_ms(10);
			if (SW_PROG)
			{
				sendProgCmd();
			}
			//wait until button is released, to avoid sending unintentional commands
			while(SW_PROG){}
			_delay_ms(200);
		}

	}
	return 0;
}

/*
 * Send the programming command, using the current dip-switch setting
 */
void sendProgCmd()
{
	const unsigned char id = readDip();
	//Only send prog-command if valid address
	if (id>1)
	{
		setLed(1,ON);
		send_packet(RC5_ADR_PROGRAMMING,id<<1,PROGLED);
		_delay_ms(300);
		setLed(1,OFF);
		_delay_ms(300);
	}
	else
	{
		//Warn the user if he/she uses id 0 or 1
		setLed(1,ON);
		_delay_ms(300);
		setLed(1,OFF);
		_delay_ms(300);
		setLed(1,ON);
		_delay_ms(300);
		setLed(1,OFF);
		_delay_ms(300);
	}
}

/*
 * Sends the start command using the current dip-switch setting
 * The command is repeated at frequency set in main.h intervals "repeats" number of times
 * Repeats = 0 gives one message
 */
void sendStart(unsigned char repeats)
{
	const unsigned char id = readDip();
	if(id==1)
	{
		//Warn the user if he/she uses id 1
		setLed(3,ON);
		_delay_ms(300);
		setLed(3,OFF);
		_delay_ms(300);
		setLed(3,ON);
		_delay_ms(300);
		setLed(3,OFF);
		_delay_ms(300);
	}
	else
	{
		setLed(3,ON);
		repeats++;
		while(repeats)
		{
			setLed(4,ON);
			//Home-mode
			if(id==0)
			{
				send_packet(RC5_ADR_CAMCORDER,RC5_CMD_1,CMDLED);
			}
			else
			{
				send_packet(RC5_ADR_EXPERIMENTAL,(id<<1 | 0b00000001),CMDLED);
			}
			setLed(4,OFF);
			_delay_ms(REPETITION_FREQ);	//According to the standard, repetition rate should be 114ms
			repeats--;
		}
		setLed(3,OFF);
	}
}

/*
 * Sends the stop command using the current dip-switch setting
 * The command is repeated at a frequency set in main.h intervals "repeats" number of times
 * Repeats = 0 gives one message
 */
void sendStop(unsigned char repeats)
{
	const unsigned char id = readDip();
	if(id==1)
	{
		//Warn the user if he/she uses id 1
		setLed(2,ON);
		_delay_ms(300);
		setLed(2,OFF);
		_delay_ms(300);
		setLed(2,ON);
		_delay_ms(300);
		setLed(2,OFF);
		_delay_ms(300);
	}
	else
	{
		setLed(2,ON);
		repeats++;
		while(repeats)
		{
			setLed(4,ON);
			//Home-mode
			if(id==0)
			{
				send_packet(RC5_ADR_CAMCORDER,RC5_CMD_2,CMDLED);
			}
			else
			{
				send_packet(RC5_ADR_EXPERIMENTAL,(id<<1 & 0b11111110),CMDLED);
			}
			setLed(4,OFF);
			_delay_ms(REPETITION_FREQ);
			repeats--;
		}
		setLed(2,OFF);
	}

}

/*
 * Prepares the remote for power-down mode (to save power)
 * Turns off more or less everything, thus drawing less than 1µA in power-down.
 * When it wakes up, it checks the battery voltage and signals
 * a low-battery state by blinking all leds repeatedly, then going back to sleep
 * Please note that no command will be sent in a low-battery state
 */
void goToSleep()
{
	//Turn off the following modules: all
	PRR |= _BV(PRTWI) | _BV(PRTIM2) | _BV(PRTIM0) | _BV(PRTIM1) | _BV(PRSPI) | _BV(PRUSART0) | _BV(PRADC);
	//Disconnect pullups on dipswitch
	PORTC &= 0b11000001;
	//Enable interrupt on pinchange PINC0, PIND1 and int0 (low-level interrupt)
	EICRA &= ~_BV(ISC00) & ~_BV(ISC01);
	EIMSK |= _BV(INT0);
	PCICR |= _BV(PCIE1) | _BV(PCIE2);
	PCMSK2 |= _BV(PCINT17);
	PCMSK1 |= _BV(PCINT8);

	sei();
	//Sleep (power down)
	SMCR |= _BV(SM1) | _BV(SE);
	asm("sleep");
	SMCR &= ~_BV(SE);
	cli();

	/*
	 * The voltage measurement is a little weird
	 * By using the battery voltage directly as vcc to the microcontroller
	 * one can measure the internal 1.1V reference, using vcc as analog reference voltage
	 * and thereby indirectly measure the battery voltage.
	 * The ADC value will increase as the battery voltage drops, hence the if(ADCH > LOW_VOLTAGE)
	 * If the battery is fine, it continues based on the button pressed
	 */
	PRR &= ~_BV(PRADC);
	ADMUX |= _BV(REFS0) | _BV(ADLAR) | 0b00001110;
	ADCSRA |= _BV(ADEN);
	//Wait for internal 1.1V reference to stabilize
	_delay_ms(5);
	ADCSRA |= _BV(ADSC);
	while(!(ADCSRA & _BV(ADIF))){}
	ADCSRA |= _BV(ADIF);

	if (ADCH > LOW_VOLTAGE)
	{
		setAllLeds(ON);
		_delay_ms(300);
		setAllLeds(OFF);
		_delay_ms(300);
		setAllLeds(ON);
		_delay_ms(300);
		setAllLeds(OFF);
		_delay_ms(300);
		setAllLeds(ON);
		_delay_ms(300);
		setAllLeds(OFF);
		_delay_ms(300);
		//This might cause a stack overflow if a button is pressed repeatedly in a battery-low state
		//However, this needs something like 100-200 button presses.
		goToSleep();
	}
	//Restore Timer0 and 2, set pullups on dip switch
	PRR &= ~_BV(PRTIM2) & ~_BV(PRTIM0);
	PORTC |= 0b001111110;
}

//Wake up on start button
ISR(PCINT1_vect)
{
	buttonPressed = START;
}

//Wake up on prog button
ISR(PCINT2_vect)
{
	buttonPressed = PROG;
}

//Wake up on stop button, low level
ISR(INT0_vect)
{
	buttonPressed = STOP;
}
