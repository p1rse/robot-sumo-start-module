/*
 * start.h
 *
 *  Created on: 20 jan 2012
 *      Author: benjamin
 */

#ifndef START_H_
#define START_H_

/*
 * Settings
 */
// Home_mode or competition_mode?
#define IS_COMP_MODE			1

// Home-mode commands
#define START_HOME_START		RC5_CMD_1
#define START_HOME_STOP			RC5_CMD_2

// Default STOP command (before something else is programmed)
#define START_DEFAULT_STOP_CMD	0x04

// IR address for competition mode
#define START_ADDRESS_COMP		RC5_ADR_EXPERIMENTAL

// IR address for programming
#define START_ADDRESS_PROG		0x0B

// Delay between STOPPED_SAFE and STOPPED in 1s/100
#define START_STOP_DELAY		100

/*
 * States
 */
#define START_STATE_POWER_ON		0
#define START_STATE_STARTED			1
#define START_STATE_STOPPED_SAFE	2
#define START_STATE_STOPPED			3

/*
 * Non-volatile data
 */
typedef struct {
	char state;
	char stop_cmd;
} START_DATA;

/*
 * Functions
 */
char start_get_state();
char start_get_saved_state();
char start_get_stop_cmd();
void start_set_home_mode();
void start_set_competition_mode();
START_DATA* start_get_saved_data();
void start_timerfunc();
char start_init(void(*save_func)(START_DATA *data),
				void(*load_func)(START_DATA *data),
				void(*programming_done)(char stop_cmd),
				void(*state_change_func)(char state));

#endif /* START_H_ */
