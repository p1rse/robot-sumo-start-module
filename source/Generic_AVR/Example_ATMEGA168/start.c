/*
 * start.c
 *
 *  Created on: 20 jan 2012
 *      Author: benjamin
 */

#include "start.h"
#include "ir.h"

/*
 * Variables
 */
volatile START_DATA start_save_data;
volatile char start_state = START_STATE_POWER_ON;

/*
 * Private functions
 */
void start_irdata_handler(IRDATA *data);
void start_state_changed_notify();

/*
 * Function pointers for user application functions
 */
static void(*start_save_func)(START_DATA *data) = 0;
static void(*start_load_func)(START_DATA *data) = 0;
static void(*start_state_change_func)(char state) = 0;
static void(*start_programming_done_func)(char new_stop_cmd) = 0;

/*
 * Get current state from start module. Can be used
 * when state_change_func is not provided when init
 * is called.
 */
char start_get_state() {
	return start_state;
}

char start_get_saved_state() {
	return start_save_data.state;
}

char start_get_stop_cmd() {
	return start_save_data.stop_cmd;
}

START_DATA* start_get_saved_data() {
	return (START_DATA*)&start_save_data;
}

/*
 * Call this function at 100Hz
 */
void start_timerfunc() {
	static unsigned int time = START_STOP_DELAY;

	if (start_state == START_STATE_STOPPED_SAFE) {
		if (time) {
			time--;

			if (!time) {
				start_state = START_STATE_STOPPED;
				start_save_data.state = START_STATE_POWER_ON;
				start_state_changed_notify();
			}
		}
	} else {
		time = START_STOP_DELAY;
	}
}

/*
 * Initialize start code
 * save_func - Pointer to function to save data (mandatory)
 * load_func - Pointer to function to load data (mandatory)
 * programming_done_func - Function to be called when programming is done (can be left as 0)
 * state_change_func - Function to be called when the state changes (can be left as 0)
 */
char start_init(void(*save_func)(START_DATA *data),
				void(*load_func)(START_DATA *data),
				void(*programming_done_func)(char new_stop_cmd),
				void(*state_change_func)(char state)) {

	if (!save_func || !load_func) {
		return -1;
	}

	// Set up function pointers
	start_save_func = save_func;
	start_load_func = load_func;
	start_programming_done_func = programming_done_func;
	start_state_change_func = state_change_func;

	// Load data from user-provided function
	start_load_func((START_DATA*)&start_save_data);

	// Make sure state is valid
	if (start_save_data.state != START_STATE_POWER_ON &&
		start_save_data.state != START_STATE_STARTED &&
		start_save_data.state != START_STATE_STOPPED_SAFE &&
		start_save_data.state != START_STATE_STOPPED) {
		start_save_data.state = START_STATE_POWER_ON;
		// Assume stop cmd is invalid as well, so set the default one
		start_save_data.stop_cmd = START_DEFAULT_STOP_CMD;
	}

	start_state = start_save_data.state;

	// Initialize IR module
	ir_init();
	ir_set_data_handler(start_irdata_handler);

	// Notify user about state update
	if (start_state_change_func) {
		start_state_change_func(start_state);
	}

	return 0;
}

void start_state_changed_notify() {
	if (start_state_change_func) {
		start_state_change_func(start_state);
	}
	
	start_save_func((START_DATA*)&start_save_data);
}

/*
 * Called by the IR driver when new data arrives
 */
void start_irdata_handler(IRDATA *data) {
	if (start_state == START_STATE_STOPPED || start_state == START_STATE_STOPPED_SAFE) {
		return;
	}

	if (data->address_low == START_ADDRESS_PROG) {
		start_save_data.stop_cmd = data->command & 0b11111110;
		start_save_data.state = START_STATE_POWER_ON;
		start_state = START_STATE_POWER_ON;

		start_save_func((START_DATA*)&start_save_data);

		if (start_programming_done_func) {
			start_programming_done_func(start_save_data.stop_cmd);
		}

		if (start_state_change_func) {
			start_state_change_func(start_state);
		}
	} else {
		switch (start_save_data.state) {
		case START_STATE_POWER_ON:
			if (IS_COMP_MODE) {
				if (data->address_low == RC5_ADR_EXPERIMENTAL) {
					if (data->command == (start_save_data.stop_cmd + 1)) {
						// Update state
						start_save_data.state = START_STATE_STARTED;
						start_state = START_STATE_STARTED;
						start_state_changed_notify();
					} else if (data->command == start_save_data.stop_cmd) {
						// Update state
						start_save_data.state = START_STATE_STOPPED_SAFE;
						start_state = START_STATE_STOPPED_SAFE;
						start_state_changed_notify();
					}
				}
			} else {
				if (data->command == START_HOME_START) {
					// Update state
					start_save_data.state = START_STATE_STARTED;
					start_state = START_STATE_STARTED;
					start_state_changed_notify();
				} else if (data->command == START_HOME_STOP) {
					// Update state
					start_save_data.state = START_STATE_STOPPED_SAFE;
					start_state = START_STATE_STOPPED_SAFE;
					start_state_changed_notify();
				}
			}
			break;

		case START_STATE_STARTED:
			if (IS_COMP_MODE) {
				if (data->address_low == RC5_ADR_EXPERIMENTAL) {
					if (data->command == start_save_data.stop_cmd) {
						// Update state
						start_save_data.state = START_STATE_STOPPED_SAFE;
						start_state = START_STATE_STOPPED_SAFE;
						start_state_changed_notify();
					}
				}
			} else {
				if (data->command == START_HOME_STOP) {
					// Update state
					start_save_data.state = START_STATE_STOPPED_SAFE;
					start_state = START_STATE_STOPPED_SAFE;
					start_state_changed_notify();
				}
			}
			break;
		}
	}
}
