/*
 * configuration_terminal.c
 *
 *  Created on: 28.11.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "configuration_terminal.h"

#include <util/delay.h>
#include <stdio.h>

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
typedef enum {
	STATE_MAIN_MENU,
	STATE_SETPID,
	STATE_TARA_ACCEL
} configuration_terminal_state_t;

static configuration_terminal_state_t current_state = STATE_MAIN_MENU;

//-------------------Defines--------------------//

/* local function declarations  */
void configuration_terminal_state_main_menu(void);
void configuration_terminal_state_setPID(void);
void configuration_terminal_state_tata_acceleration(void);

/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{

	for(;;) {

		switch(current_state) {

			case STATE_MAIN_MENU:
				configuration_terminal_state_main_menu();
			break;

			case STATE_SETPID:
				configuration_terminal_state_setPID();
			break;

			case STATE_TARA_ACCEL:
				configuration_terminal_state_tata_acceleration();
			break;
		}

		_delay_ms(2000.0);
	}
}


void configuration_terminal_state_main_menu(void)
{
	printf("MAIN MENU\n");

	current_state = STATE_SETPID;
}

void configuration_terminal_state_setPID(void)
{
	printf("MAIN SET PID\n");

	current_state = STATE_TARA_ACCEL;
}

void configuration_terminal_state_tata_acceleration(void)
{
	printf("MAIN TARA ACCELERATIONSENSOR\n");

	current_state = STATE_MAIN_MENU;
}
