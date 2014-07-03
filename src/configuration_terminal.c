/*
 * configuration_terminal.c
 *
 *  Created on: Jun 29, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "configuration_terminal.h"

/* * system headers              * */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* * local headers               * */
#include "lib/uart.h"

#include "lib/twi_master.h"

#include "configuration_storage.h"
#include "motionsensor.h"
#include "vt100.h"

/* *** DEFINES ************************************************************** */
#define INPUT_BUFFER_MAX	40

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
typedef enum {
	STATE_READ_INPUT,
	STATE_PRINT_HELP,
	STATE_PRINT_CONFIG,
	STATE_SET_PARAMETER,
	STATE_SET_ZERO_POINT,
	STATE_SAVE_CONFIG,
	STATE_FINAL,
	STATE_NULL
} configuration_terminal_state_t;

/* * local objects               * */
static configuration_terminal_state_t current_state;
static configuration_terminal_state_t next_state;

static char input_buffer[INPUT_BUFFER_MAX];

/* * local function declarations * */
static void configuration_terminal_state_read_input(void);
static void configuration_terminal_state_print_help(void);
static void configuration_terminal_state_print_config(void);
static void configuration_terminal_state_set_parameter(void);
static void configuration_terminal_state_set_zero_point(void);
static void configuration_terminal_state_save_configuration(void);

/* *** FUNCTION DEFINITIONS ************************************************* */

void configuration_terminal_state_machine(void)
{
	current_state = STATE_PRINT_CONFIG;
	next_state = STATE_NULL;

 	while(current_state != STATE_FINAL) {
		switch(current_state) {

			case STATE_READ_INPUT:
				configuration_terminal_state_read_input();
			break;

			case STATE_PRINT_HELP:
				configuration_terminal_state_print_help();
			break;

			case STATE_PRINT_CONFIG:
				configuration_terminal_state_print_config();
			break;

			case STATE_SET_PARAMETER:
				configuration_terminal_state_set_parameter();
			break;

			case STATE_SET_ZERO_POINT:
				configuration_terminal_state_set_zero_point();
			break;

			case STATE_SAVE_CONFIG:
				configuration_terminal_state_save_configuration();
			break;

			default:
			break;
		}

		current_state = next_state;
		next_state = STATE_NULL;
 	}
}

void configuration_terminal_state_print_help(void)
{
	printf("valid commands:\n");
	printf("s[value],[integer] - set selected parameter to integer\n");
	printf("    [value]   = 'p', 'i', 'd', 's', 'm'\n");
	printf("    [integer] = > 0 and < INT16_MAX'\n");
	printf("z - set new zero point for acceleration\n");
	printf("? - show this help\n");
	printf("c - show current configuration\n");
	printf("q - quit terminal and write changes to eeprom\n");


	next_state = STATE_READ_INPUT;
}
void configuration_terminal_state_print_config(void)
{
	acceleration_t accel_offset;
	configuration_storage_get_acceleration_offset(&accel_offset);

	printf("pid.p: %i\n",configuration_storage_get_p_factor());
	printf("pid.i: %i\n",configuration_storage_get_i_factor());
	printf("pid.d: %i\n",configuration_storage_get_d_factor());
	printf("pid.scaling: %u\n",configuration_storage_get_scalingfactor());
	printf("position_multiplier: %u\n",configuration_storage_get_position_multiplier());
	printf("accel_offset.x: %i\n",accel_offset.x);
	printf("accel_offset.y: %i\n",accel_offset.y);
	printf("accel_offset.z: %i\n",accel_offset.z);
	printf("gyroscope.bias: 0.0\n");

	next_state = STATE_READ_INPUT;
}


void configuration_terminal_state_read_input(void)
{
	//ENTRY
	//DO

	printf(" > ");

	vt100_clear_input_buffer();
	vt100_get_string(input_buffer,INPUT_BUFFER_MAX);

	printf("%s\n",input_buffer);

	if(input_buffer[0] == 'c') {
		next_state = STATE_PRINT_CONFIG;
	} else if (input_buffer[0] == '?') {
		next_state = STATE_PRINT_HELP;
	} else if (input_buffer[0] == 's') {
		next_state = STATE_SET_PARAMETER;
	} else if (input_buffer[0] == 'z') {
		next_state = STATE_SET_ZERO_POINT;
	} else if (input_buffer[0] == 'q') {
		next_state = STATE_SAVE_CONFIG;
	} else {
		next_state = STATE_READ_INPUT;
	}
	vt100_clear_input_buffer();
}

void configuration_terminal_state_set_parameter(void)
{
	char value_string[10];
	char *c = input_buffer;

	char command = input_buffer[1];
	char seperator = input_buffer[2];


	uint16_t value_uint16 = 0;

	uint8_t i = 0;

	//parsing string to integer
	if(strlen(input_buffer) > 3 && seperator == ',') {
		c += 3;

		while(*c != '\0') {
			value_string[i] = *c;
			c++;
			i++;
		}
		value_string[i] = '\0';

		sscanf(value_string, "%u", &value_uint16);

		//should always non negative ...
		if(value_uint16 < 0) {
			value_uint16 = 0;
		}

		//and never greater INT16_MAX
		if(value_uint16 > INT16_MAX) {
			value_uint16 = INT16_MAX;
		}

		if(command == 'p') {
			configuration_storage_set_p_factor(value_uint16);
		} else if(command == 'i') {
			configuration_storage_set_i_factor(value_uint16);
		} else if(command == 'd') {
			configuration_storage_set_d_factor(value_uint16);
		} else if(command == 's') {
			configuration_storage_set_scalingfactor(value_uint16);
		} else if(command == 'm') {
			configuration_storage_set_position_multiplier(value_uint16);
		}

		printf("OK\n");
	} else {
		printf("invalid input\n");
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_zero_point(void)
{
	acceleration_t acceleration;

	//Perform calibration
	motionsensor_set_zero_point();

	//Read and save the offset from the sensor
	motionsensor_get_acceleration_offset(&acceleration);
	configuration_storage_set_acceleration_offset(&acceleration);

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_save_configuration(void)
{
	configuration_storage_save_configuration();
	next_state = STATE_FINAL;
}
