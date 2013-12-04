/*
 * system_controller.c
 *
 *  Created on: Dec 4, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "system_controller.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

typedef enum {
	STATE_INIT_COMMUNICATION_INTERFACES,
	STATE_LOAD_SETTINGS,
	STATE_WAITING_FOR_USER_INTERRUPT,
	STATE_RUN_CONFIGURATION_TERMINAL,
	STATE_INIT_PID_CONTROLLER,
	STATE_INIT_TIMER,
	STATE_RUN_PID_CONTROLLER,
	STATE_INIT,
	STATE_FINAL,
	STATE_NULL
} system_controller_state_t;


static system_controller_state_t current_state = STATE_INIT_COMMUNICATION_INTERFACES;
static system_controller_state_t next_state = STATE_NULL;

/* local function declarations  */


// states
void system_controller_state_init_communicastion_interfaces(void);
void system_controller_state_load_settings(void);
void system_controller_state_waiting_for_user_interrupt(void);
void system_controller_state_run_configuration_terminal(void);
void system_controller_state_init_pid_controller(void);
void system_controller_state_init_timer(void);
void system_controller_state_run_pid_controller(void);
void system_controller_state_null(void);


/* *** FUNCTION DEFINITIONS ************************************************** */
void system_controller_state_state_machine(void)
{

	while(current_state != STATE_FINAL) {
		switch(current_state) {

			case STATE_INIT_COMMUNICATION_INTERFACES:
			break;

			case STATE_LOAD_SETTINGS:
			break;

			case STATE_WAITING_FOR_USER_INTERRUPT:
			break;

			case STATE_RUN_CONFIGURATION_TERMINAL:
			break;

			case STATE_INIT_PID_CONTROLLER:
			break;

			case STATE_INIT_TIMER:
			break;

			case STATE_INIT:
			break;

			case STATE_FINAL:
			break;

			case STATE_NULL:
			break;
		}
	}
}

void system_controller_state_null(void)
{
	/* *** ENTRY *** */

	/* **** DO ***** */

	/* *** EXIT **** */
}
