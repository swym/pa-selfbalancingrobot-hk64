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
#include <avr/eeprom.h>

#include "configuration_setting.h"
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


/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{
 	while(current_state != STATE_FINAL) {
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
				//TODO: run system with current settings
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
	uint8_t i = 0;

	vt100_clear_all();


	printf("           P      I      D      F      M     Ox     Oy     Oz\n\n");


	for(i = 0;i < CONFIGURATION_SETTING_COUNT;i++) {
		printf(" [%u] - \"%s\"\n", i, configuration_setting_data[i].comment);
		printf("       %5u  %5u  %5u  %5u  %5u  %5u  %5u  %5u\n\n",
				configuration_setting_data[i].pid_p_factor,
				configuration_setting_data[i].pid_i_factor,
				configuration_setting_data[i].pid_d_factor,
				configuration_setting_data[i].pid_scalingfactor,
				configuration_setting_data[i].position_multiplier,
				configuration_setting_data[i].acceleration_offset.x,
				configuration_setting_data[i].acceleration_offset.y,
				configuration_setting_data[i].acceleration_offset.z);
	}

	printf("Index of current setting is: %u\n", configuration_setting_current_index);

	//DO

	configuration_setting_current_index = vt100_get_integer(
						configuration_setting_current_index,
						0,
						CONFIGURATION_SETTING_COUNT);


	//EXIT

	next_state = STATE_MAIN_MENU;
}


void configuration_terminal_state_write_settings(void)
{
	uint8_t i;


	//ENTRY
	vt100_clear_all();

	//DO
	printf("Writing settings to eeprom....\n");

	for(i = 0;i < CONFIGURATION_SETTING_COUNT;i++) {

		eeprom_write_block(&configuration_setting_data[i],
						   &configuration_setting_data_eeprom[i],
						   sizeof(configuration_setting_t));
	}

	eeprom_write_byte(&configuration_setting_current_index_eeprom,
			configuration_setting_current_index);

	//EXIT
	next_state = STATE_MAIN_MENU;
}

void configuration_terminal_state_export_settings(void)
{
	uint8_t i;

	//ENTRY
	vt100_clear_all();

	//DO
	for(i = 0;i < CONFIGURATION_SETTING_COUNT;i++) {
		printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,\"%s\"\n",
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
	}

	printf("\n\nPress any key to go back to main menu.\n");

	//wait for user input; but don't use it
	vt100_get_choice();

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
	printf("[C] - \"%s\"\n\n", configuration_setting_data[configuration_setting_current_index].comment);


	printf("    PID Controller                          Accelerationsensor\n\n");
	printf("[P] - Proportional Parameter : %5u    [M] - Position Multiplier    : %5u\n",
			configuration_setting_data[configuration_setting_current_index].pid_p_factor,
			configuration_setting_data[configuration_setting_current_index].position_multiplier);

	printf("[I] - Integral Parameter     : %5u    [O] - Set Offset           X : %5u\n",
			configuration_setting_data[configuration_setting_current_index].pid_i_factor,
			configuration_setting_data[configuration_setting_current_index].acceleration_offset.x);

	printf("[D] - Derivative Parameter   : %5u                               Y : %5u\n",
			configuration_setting_data[configuration_setting_current_index].pid_d_factor,
			configuration_setting_data[configuration_setting_current_index].acceleration_offset.y);

	printf("[F] - Factor                 : %5u                               Z : %5u\n",
			configuration_setting_data[configuration_setting_current_index].pid_scalingfactor,
			configuration_setting_data[configuration_setting_current_index].acceleration_offset.z);

	printf("\n\n    Settings\n\n[S] - Select current setting\n[W] - Write settings to EEPROM\n[E] - Export to csv\n");
	printf("\n\n    Run\n\n[R] - Run system with current configuration\n");

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
		} else if(choice == 'W') {
			next_state = STATE_WRITE_SETTINGS;
		} else if(choice == 'E') {
			next_state = STATE_EXPORT_SETTINGS;
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
	printf("Current value: %u\n\n", configuration_setting_data[configuration_setting_current_index].pid_p_factor);


	// DO
	configuration_setting_data[configuration_setting_current_index].pid_p_factor =
			vt100_get_integer(configuration_setting_data[configuration_setting_current_index].pid_p_factor,
							  0,
							  UINT16_MAX);
	next_state = STATE_MAIN_MENU;

	// EXIT

}


void configuration_terminal_state_PID_set_I(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== CHANGE INGETRAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", configuration_setting_data[configuration_setting_current_index].pid_i_factor);


	// DO
	configuration_setting_data[configuration_setting_current_index].pid_i_factor =
			vt100_get_integer(configuration_setting_data[configuration_setting_current_index].pid_i_factor,
							  0,
							  UINT16_MAX);
	next_state = STATE_MAIN_MENU;
	// EXIT

}


void configuration_terminal_state_PID_set_D(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== CHANGE DERIVATIVE PARAMETER ===\n\n");
	printf("Current value: %u\n\n", configuration_setting_data[configuration_setting_current_index].pid_d_factor);


	// DO
	configuration_setting_data[configuration_setting_current_index].pid_d_factor =
			vt100_get_integer(configuration_setting_data[configuration_setting_current_index].pid_d_factor,
							  0,
							  UINT16_MAX);
	next_state = STATE_MAIN_MENU;


	// EXIT

}



void configuration_terminal_state_PID_set_scalingfactor(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== CHANGE PID SCALING FACTOR ===\n\n");
//	printf("Current value: %u\n\n", pid_scaling_factor);

	// DO
	configuration_setting_data[configuration_setting_current_index].pid_scalingfactor = vt100_get_integer(
			configuration_setting_data[configuration_setting_current_index].pid_scalingfactor, 1, UINT16_MAX);



	next_state = STATE_MAIN_MENU;


	// EXIT
}


void configuration_terminal_state_accelerationsensor_set_zero(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== ACCEL SET ZERO MENU ===\n");

	printf("Current position is new zero!");

	acceleration_t accel;

	// DO

	//Perform calibration
	acceleration_calibrate_offset();

	//Read and save the offset from the sensor
	acceleration_get_offset(&accel);

	configuration_setting_data[configuration_setting_current_index].acceleration_offset.x = accel.x;
	configuration_setting_data[configuration_setting_current_index].acceleration_offset.y = accel.y;
	configuration_setting_data[configuration_setting_current_index].acceleration_offset.z = accel.z;

	next_state = STATE_MAIN_MENU;
	// EXIT

}


void configuration_terminal_state_accelerationsensor_set_scalingfactor(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== ACCELERATIONSENSOR SCALING FACTOR ===\n\n");
	printf("Current value: %u\n\n", configuration_setting_data[configuration_setting_current_index].position_multiplier);

	// DO
	configuration_setting_data[configuration_setting_current_index].position_multiplier =
			vt100_get_integer(
						configuration_setting_data[configuration_setting_current_index].position_multiplier,
						1,
						UINT16_MAX);

	next_state = STATE_MAIN_MENU;

	// EXIT
}


void configuration_terminal_state_edit_comment(void)
{
	// ENTRY
	vt100_clear_all();

	printf("=== EDIT COMMENT ===\n\n");

	printf("Current value: \"%s\"\n\n", configuration_setting_data[configuration_setting_current_index].comment);

	// DO
	vt100_get_string(configuration_setting_data[configuration_setting_current_index].comment,
		CONFIGURATION_SETTING_COMMENT_LENGTH);



	next_state = STATE_MAIN_MENU;


	// EXIT
}


