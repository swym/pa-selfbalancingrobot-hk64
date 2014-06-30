/*
 * configuration_terminal.c
 *
 *  Created on: Dec 5, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "configuration_terminal.h"

#include <stdio.h>
#include <stdint.h>

#include "configuration_manager.h"
#include "vt100.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
typedef enum {
	STATE_MAIN_MENU,
	STATE_SELECT_SETTINGS,
	STATE_WRITE_SETTINGS,
	STATE_EXPORT_SETTINGS,
	STATE_PID_SET_P,
	STATE_PID_SET_I,
	STATE_PID_SET_D,
	STATE_PID_SET_SCALINGFACTOR,
	STATE_ACCELERATIONSENSOR_SET_ZERO,
	STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR,
	STATE_EDIT_COMMENT,
	STATE_FINAL,
	STATE_NULL
} configuration_terminal_state_t;

static configuration_terminal_state_t current_state = STATE_MAIN_MENU;
static configuration_terminal_state_t next_state = STATE_NULL;

/* local function declarations  */

/* states */
void configuration_terminal_state_select_settings(void);
void configuration_terminal_state_write_settings(void);
void configuration_terminal_state_export_settings(void);
void configuration_terminal_state_main_menu(void);
void configuration_terminal_state_PID_set_P(void);
void configuration_terminal_state_PID_set_I(void);
void configuration_terminal_state_PID_set_D(void);
void configuration_terminal_state_PID_set_scalingfactor(void);
void configuration_terminal_state_edit_comment(void);
void configuration_terminal_state_accelerationsensor_set_zero(void);
void configuration_terminal_state_accelerationsensor_set_scalingfactor(void);
void configuration_terminal_state_final(void);


/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{


 	while(current_state != STATE_NULL) {
		switch(current_state) {

			case STATE_SELECT_SETTINGS:
				configuration_terminal_state_select_settings();
			break;

			case STATE_WRITE_SETTINGS:
				configuration_terminal_state_write_settings();
			break;

			case STATE_EXPORT_SETTINGS:
				configuration_terminal_state_export_settings();
			break;

			case STATE_MAIN_MENU:
				configuration_terminal_state_main_menu();
			break;

			case STATE_PID_SET_P:
				configuration_terminal_state_PID_set_P();
			break;

			case STATE_PID_SET_I:
				configuration_terminal_state_PID_set_I();
			break;

			case STATE_PID_SET_D:
				configuration_terminal_state_PID_set_D();
			break;

			case STATE_PID_SET_SCALINGFACTOR:
				configuration_terminal_state_PID_set_scalingfactor();
			break;

			case STATE_ACCELERATIONSENSOR_SET_ZERO:
				configuration_terminal_state_accelerationsensor_set_zero();
			break;

			case STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR:
				configuration_terminal_state_accelerationsensor_set_scalingfactor();
			break;

			case STATE_EDIT_COMMENT:
				configuration_terminal_state_edit_comment();
			break;

			case STATE_FINAL:
				configuration_terminal_state_final();
			break;
			case STATE_NULL:
				//TODO: Implement Null state
			break;
		}

		current_state = next_state;
		next_state = STATE_NULL;
	}
}

void configuration_terminal_state_select_settings(void)
{
	//ENTRY
//	uint8_t i = 0;

	uint8_t new_index;

	vt100_clear_all();


	configuration_manager_print_all_configs();

	new_index = vt100_get_integer(configuration_manager_get_current_config_index(),
			0,
			CONFIGURATION_MANAGER_CONFIG_COUNT - 1);

	configuration_manager_select_config(new_index);

	//EXIT

	next_state = STATE_MAIN_MENU;
}


void configuration_terminal_state_write_settings(void)
{
	//TODO
/*	//ENTRY
	vt100_clear_all();

	//DO
	printf("Writing settings to eeprom....\n");

	configuration_manager_write_config();

	//EXIT
	 *
	 */
	next_state = STATE_MAIN_MENU;
}

void configuration_terminal_state_export_settings(void)
{/*
	uint8_t i;

	//ENTRY
	vt100_clear_all();

	//DO
	for(i = 0;i < CONFIGURATION_MANAGER_CONFIG_COUNT;i++) {

		printf("%u,%u,%u,%u,%u,%u,%i,%i,%i,\"%s\"\n",
				i,
				configuration_setting_data[i].pid_p_factor,
				configuration_setting_data[i].pid_i_factor,
				configuration_setting_data[i].pid_d_factor,
				configuration_setting_data[i].pid_scalingfactor,
				configuration_setting_data[i].position_multiplier,
				configuration_setting_data[i].acceleration_offset.x,
				configuration_setting_data[i].acceleration_offset.y,
				configuration_setting_data[i].acceleration_offset.z,
				configuration_setting_data[i].comment);

		printf("tba\n");
	}

	printf("\n\nPress any key to go back to main menu.\n");

	//wait for user input; but don't use it

	vt100_get_choice();
	*/

	//EXIT
	next_state = STATE_MAIN_MENU;
}


void configuration_terminal_state_main_menu(void)
{
	char choice;

	// ENTRY
	vt100_clear_all();

	//Greeting

	printf("                          [Current Configuration]\n\n");
	printf("[C] - \"%s\"\n\n", configuration_manager_current_config_get_comment());


	printf("    PID Controller                          Accelerationsensor\n\n");
	printf("[P] - Proportional Parameter : %5i    [M] - Position Multiplier    : %5u\n",
			configuration_manager_current_config_get_p_factor(),
			configuration_manager_current_config_get_position_multiplier());

	printf("[I] - Integral Parameter     : %5i    [O] - Set Offset           X : %5i\n",
			configuration_manager_current_config_get_i_factor(),
			configuration_manager_current_config_get_acceleration_offset()->x);

	printf("[D] - Derivative Parameter   : %5u                               Y : %5i\n",
			configuration_manager_current_config_get_d_factor(),
			configuration_manager_current_config_get_acceleration_offset()->y);

	printf("[F] - Factor                 : %5u                               Z : %5i\n",
			configuration_manager_current_config_get_scalingfactor(),
			configuration_manager_current_config_get_acceleration_offset()->z);

	printf("\n\n    Settings\n\n[S] - Select Configuration\n");
	printf("\n\n    Run\n\n[R] - Save current configuration and run system\n");

	// DO
	//Waiting for users choice
	do {
		choice = vt100_get_choice();

		if(choice == 'P') {
			next_state = STATE_PID_SET_P;
		} else if(choice == 'I') {
			next_state = STATE_PID_SET_I;
		} else if(choice == 'D') {
			next_state = STATE_PID_SET_D;
		} else if(choice == 'F') {
			next_state = STATE_PID_SET_SCALINGFACTOR;
		} else if(choice == 'M') {
			next_state = STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR;
		} else if(choice == 'O') {
			next_state = STATE_ACCELERATIONSENSOR_SET_ZERO;
		} else if(choice == 'C') {
			next_state = STATE_EDIT_COMMENT;
		} else if(choice == 'S') {
			next_state = STATE_SELECT_SETTINGS;
//		} else if(choice == 'W') {
//			next_state = STATE_WRITE_SETTINGS;
//		} else if(choice == 'E') {
//			next_state = STATE_EXPORT_SETTINGS;
		} else if(choice == 'R') {
			next_state = STATE_FINAL;
		} else {
			printf("Invalid choice! Please retry:\n");
			choice = 0;
		}

	} while(choice == 0);

	// EXIT

}



void configuration_terminal_state_PID_set_P(void)
{
	// ENTRY
	vt100_clear_all();


	printf("=== CHANGE PROPORTIONAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", configuration_manager_current_config_get_p_factor());


	// DO
	configuration_manager_current_config_set_p_factor(vt100_get_integer(
			configuration_manager_current_config_get_p_factor(),
			0,
			UINT16_MAX));

	next_state = STATE_MAIN_MENU;

	// EXIT

}


void configuration_terminal_state_PID_set_I(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== CHANGE INGETRAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", configuration_manager_current_config_get_i_factor());


	// DO
	configuration_manager_current_config_set_i_factor(vt100_get_integer(
			configuration_manager_current_config_get_i_factor(),
			0,
			UINT16_MAX));

	next_state = STATE_MAIN_MENU;
	// EXIT

}


void configuration_terminal_state_PID_set_D(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== CHANGE DERIVATIVE PARAMETER ===\n\n");
	printf("Current value: %u\n\n", configuration_manager_current_config_get_d_factor());


	// DO
	configuration_manager_current_config_set_d_factor(vt100_get_integer(
			configuration_manager_current_config_get_d_factor(),
			0,
			UINT16_MAX));

	next_state = STATE_MAIN_MENU;


	// EXIT

}



void configuration_terminal_state_PID_set_scalingfactor(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== CHANGE PID SCALING FACTOR ===\n\n");
	printf("Current value: %u\n\n", configuration_manager_current_config_get_scalingfactor());

	// DO
	configuration_manager_current_config_set_scalingfactor(vt100_get_integer(
			configuration_manager_current_config_get_scalingfactor(),
			1,
			UINT16_MAX));//TODO

	next_state = STATE_MAIN_MENU;


	// EXIT
}


void configuration_terminal_state_accelerationsensor_set_zero(void)
{
	// ENTRY

	acceleration_t accel;

	// DO

	//Perform calibration
	accelerationsensor_calibrate_offset();

	//Read and save the offset from the sensor
	accelerationsensor_get_offset(&accel);
	configuration_manager_current_config_set_acceleration_offset(&accel);

	// EXIT

	next_state = STATE_MAIN_MENU;

}


void configuration_terminal_state_accelerationsensor_set_scalingfactor(void)
{
	//TODO Naming STATE/FUNCTION/CONFIG MANAGER

	// ENTRY
	vt100_clear_all();

	printf("=== ACCELERATIONSENSOR SCALING FACTOR ===\n\n");
	printf("Current value: %u\n\n", configuration_manager_current_config_get_position_multiplier());

	// DO
	configuration_manager_current_config_set_position_multiplier(vt100_get_integer(
			configuration_manager_current_config_get_position_multiplier(),
			1,
			UINT16_MAX));

	next_state = STATE_MAIN_MENU;

	// EXIT
}


void configuration_terminal_state_edit_comment(void)
{
	// ENTRY
	vt100_clear_all();

	char new_comment[CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH];

	printf("=== EDIT COMMENT ===\n\n");

	printf("Current value: \"%s\"\n\n", configuration_manager_current_config_get_comment());

	// DO

	vt100_get_string(new_comment, CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH);
	configuration_manager_current_config_set_comment(new_comment);

	next_state = STATE_MAIN_MENU;


	// EXIT
}

void configuration_terminal_state_final(void)
{
	//ENTRY
	vt100_clear_all();

	//DO
	if(configuration_manager_current_config_has_changed()) {

		char new_comment[CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH];

		printf("=== EDIT COMMENT ===\n\n");

		printf("Configuration has changed. Please alter comment if necessary\n\n");
		printf("Current comment: \"%s\"\n\n", configuration_manager_current_config_get_comment());

		vt100_get_string(new_comment, CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH);

		if(new_comment[0] != '\0') {
			configuration_manager_current_config_set_comment(new_comment);
		}

		vt100_clear_all();
		printf("Writing settings to eeprom....\n");
		configuration_manager_write_config();
	}

	//EXIT
	vt100_clear_all();
	next_state = STATE_NULL;
}



