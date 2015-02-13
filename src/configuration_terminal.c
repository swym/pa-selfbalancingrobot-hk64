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
#include <util/delay.h>

/* * local headers               * */
#include "lib/uart.h"

#include "lib/twi_master.h"

#include "configuration_storage.h"
#include "motionsensor.h"
#include "vt100.h"

/* *** DEFINES ************************************************************** */
#define INPUT_BUFFER_MAX			40
#define INPUT_BUFFER_PARSE_MINLEN	3

#define INPUT_BUFFER_COMMAND			0
#define INPUT_BUFFER_VALUE				1
#define INPUT_BUFFER_SEPERATOR			','
#define INPUT_BUFFER_SEPERATOR_INDEX	2

#define VALUE_BUFFER_MAX			10

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
typedef enum {
	STATE_READ_INPUT,
	STATE_PRINT_HELP,
	STATE_PRINT_CONFIG,
	STATE_SET_PID_PARAMETER,
	STATE_SET_COMPLEMENTARY_FILTER_PARAMETER,
	STATE_SET_OFFSET,
	STATE_SAVE_CONFIG,
	STATE_LIVE_DATA,
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
static void configuration_terminal_state_set_pid_parameter(void);
static void configuration_terminal_state_set_complementary_filter_parameter(void);
static void configuration_terminal_state_set_offset(void);
static void configuration_terminal_state_save_configuration(void);
static void configuration_terminal_state_live_data(void);

static bool parse_input2int16(int16_t *value, int16_t min, int16_t max);
//static bool parse_input2double(double *value, double min, double max);

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

			case STATE_SET_PID_PARAMETER:
				configuration_terminal_state_set_pid_parameter();
			break;

			case STATE_SET_COMPLEMENTARY_FILTER_PARAMETER:
				configuration_terminal_state_set_complementary_filter_parameter();
			break;

			case STATE_LIVE_DATA:
				configuration_terminal_state_live_data();
			break;

			case STATE_SET_OFFSET:
				configuration_terminal_state_set_offset();
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
	printf("valid commands:\n\n");

	printf("pp,<0 .. %d> - set proportional factor of PID controller\n", INT16_MAX);
	printf("pi,<0 .. %d> - set integral factor of PID controller\n", INT16_MAX);
	printf("pd,<0 .. %d> - set differential factor of PID controller\n\n", INT16_MAX);

	printf("ps,<0 .. %d> - set the result divider of the PID controller\n\n", INT16_MAX);

//	printf("mm,<0 .. 1024> - set the position multiplier\n\n");

	printf("ca,<0 .. 100>  - set the acceleration_factor of complementary filter\n");
	printf("cv,<0 .. 100>  - set the angularvelocity_factor of complementary filter\n");
	printf("                 [!] Sum of ca and cv must be 100 [!]\n\n");

	printf("z - set new offset for acceleration and angularvelocity\n\n");

//	printf("l - show live data\n\n");

	printf("? - print this help\n");
	printf("s - show current configuration\n\n");

	printf("x - quit terminal and DISCARD changes\n");
	printf("q - quit terminal and SAVE changes to eeprom\n");


	next_state = STATE_READ_INPUT;
}
void configuration_terminal_state_print_config(void)
{
	acceleration_t accel_offset;
	configuration_storage_get_acceleration_offset(&accel_offset);

	angularvelocity_t angvelo_offset;
	configuration_storage_get_angularvelocity_offset(&angvelo_offset);


	printf("pid.p: %i\n",configuration_storage_get_p_factor());
	printf("pid.i: %i\n",configuration_storage_get_i_factor());
	printf("pid.d: %i\n",configuration_storage_get_d_factor());
	printf("pid.scalingfactor: %u\n",configuration_storage_get_scalingfactor());
//	printf("position.multiplier: %u\n",configuration_storage_get_position_multiplier());
	printf("complementary_filter.angularvelocity_factor: %i\n",(int16_t)(configuration_storage_get_complementary_filter_angularvelocity_factor() * 100.0));
	printf("complementary_filter.acceleration_factor: %i\n",(int16_t)(configuration_storage_get_complementary_filter_acceleraton_factor() * 100.0));
	printf("motionsensor.acceleration_offset.x: %i\n",accel_offset.x);
	printf("motionsensor.acceleration_offset.y: %i\n",accel_offset.y);
	printf("motionsensor.acceleration_offset.z: %i\n",accel_offset.z);
	printf("motionsensor.angularvelocity_offset.x: %i\n",angvelo_offset.x);
	printf("motionsensor.angularvelocity_offset.y: %i\n",angvelo_offset.y);
	printf("motionsensor.angularvelocity_offset.z: %i\n",angvelo_offset.z);
	printf("enter '?' for help\n");


	next_state = STATE_READ_INPUT;
}


void configuration_terminal_state_read_input(void)
{
	//ENTRY
	//DO

	vt100_clear_input_buffer();
	UART_char_received();

	printf(" > ");

	vt100_clear_input_buffer();
	vt100_get_string(input_buffer,INPUT_BUFFER_MAX);

//	printf("%s",input_buffer);


	switch (input_buffer[INPUT_BUFFER_COMMAND]) {
		case 'p':
			next_state = STATE_SET_PID_PARAMETER;
			break;
		case 'c':
			next_state = STATE_SET_COMPLEMENTARY_FILTER_PARAMETER;
			break;
		case 'z':
			next_state = STATE_SET_OFFSET;
			break;
		case '?':
			next_state = STATE_PRINT_HELP;
			break;
		case 's':
			next_state = STATE_PRINT_CONFIG;
			break;
//		case 'l':
//			next_state = STATE_LIVE_DATA;
//			break;
		case 'q':
			next_state = STATE_SAVE_CONFIG;
			break;
		case 'x':
			next_state = STATE_FINAL;
			break;
		default:
			next_state = STATE_READ_INPUT;
			break;
	}

	vt100_clear_input_buffer();
}

void configuration_terminal_state_set_pid_parameter(void)
{
	int16_t pid_value = 0;

	//parsing string to integer
	if(parse_input2int16(&pid_value, 0, INT16_MAX)) {

		switch (input_buffer[INPUT_BUFFER_VALUE]) {
			case 'p':
				configuration_storage_set_p_factor(pid_value);
				printf("OK\n");
				break;
			case 'i':
				configuration_storage_set_i_factor(pid_value);
				printf("OK\n");
				break;
			case 'd':
				configuration_storage_set_d_factor(pid_value);
				printf("OK\n");
				break;
			case 's':
				configuration_storage_set_scalingfactor(pid_value);
				printf("OK\n");
				break;
			default:
				printf("invalid value\n");
				break;
		}
	} else {
		printf("invalid number\n");
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_complementary_filter_parameter(void)
{
	int16_t tmp_int16 = 0;
	double tmp_double = 0.0;

	switch (input_buffer[INPUT_BUFFER_VALUE]) {
		case 'a':
			if(parse_input2int16(&tmp_int16, 0, 100)) {
				tmp_double = (double)(tmp_int16) / 100.0;
				configuration_storage_set_complementary_filter_acceleraton_factor(tmp_double);
				printf("OK\n");
			} else {
				printf("invalid number\n");
			}
			break;

		case 'v':
			if(parse_input2int16(&tmp_int16, 0, 100)) {
				tmp_double = (double)(tmp_int16) / 100.0;
				configuration_storage_set_complementary_filter_angularvelocity_factor(tmp_double);
				printf("OK\n");
			} else {
				printf("invalid number\n");
			}
			break;

//		case 'm':
//			if(parse_input2int16(&tmp_int16, 0, 1024)) {
//				configuration_storage_set_position_multiplier(tmp_int16);
//				printf("OK\n");
//			} else {
//				printf("invalid number\n");
//			}
//			break;

		default:
			printf("invalid value\n");
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_offset(void)
{
	acceleration_t acceleration_offset;
	angularvelocity_t angularvelocity_offset;

	//Perform calibration
	motionsensor_set_zero_point();

	//Read and save the offset from the sensor
	motionsensor_get_acceleration_offset(&acceleration_offset);
	configuration_storage_set_acceleration_offset(&acceleration_offset);

	motionsensor_get_angularvelocity_offset(&angularvelocity_offset);
	configuration_storage_set_angularvelocity_offset(&angularvelocity_offset);

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_save_configuration(void)
{
	configuration_storage_save_configuration();
	next_state = STATE_FINAL;
}

void configuration_terminal_state_live_data(void)
{
	int16_t run = 500;

	acceleration_t acceleration;
	angularvelocity_t angularvelocity;

	while(run--) {
		printf("% 5d% 5d% 5d",acceleration.x, acceleration.y, acceleration.z);
		printf("% 5d% 5d% 5d",angularvelocity.x, angularvelocity.y, angularvelocity.z);
		printf("\n");
		_delay_ms(3);
	}

	next_state = STATE_READ_INPUT;
}

bool parse_input2int16(int16_t *value, int16_t min, int16_t max)
{
	char *input_ptr = input_buffer;		//set intput_ptr to begin of input_buffer
	char value_buffer[VALUE_BUFFER_MAX];
	uint8_t i = 0;

	//try parsing only string witch is long enough, has a seperator on the right index
	if(strlen(input_buffer) > INPUT_BUFFER_PARSE_MINLEN &&
		input_buffer[INPUT_BUFFER_SEPERATOR_INDEX] == INPUT_BUFFER_SEPERATOR) {

		input_ptr += INPUT_BUFFER_PARSE_MINLEN; //move pointer to begin of number

		//iterate over input buffer and copy chars to value_buffer
		while(*input_ptr != '\0' && i < VALUE_BUFFER_MAX) {
			value_buffer[i] = *input_ptr;
			input_ptr++;
			i++;
		}

		//add endmark
		value_buffer[i] = '\0';

		//try to parse string in value_buffer to int16_t
		// greater 0 - success; else failure
		if(sscanf(value_buffer, "%d", value)) {

			//respect max
			if(*value >= max) {
				*value = max;
			}

			//respect min
			if(*value < min) {
				*value = min;
			}

			return true;
		}
	}
	//if strlen() mismatch or  parsing of sscanf() fails return false;
	return false;
}

//FIXME Cannot parse float/double - missing lib?
/*
bool parse_input2double(double *value, double min, double max)
{
	double testval = -1.0;

	char *input_ptr = input_buffer;		//set intput_ptr to begin of input_buffer

	char value_buffer[VALUE_BUFFER_MAX];
	uint8_t i = 0;

	//try parsing only string witch is long enough, has a seperator on the right index
	if(strlen(input_buffer) > INPUT_BUFFER_PARSE_MINLEN &&
		input_buffer[INPUT_BUFFER_SEPERATOR_INDEX] == INPUT_BUFFER_SEPERATOR) {

		input_ptr += INPUT_BUFFER_PARSE_MINLEN; //move pointer to begin of number

		//iterate over input buffer and copy chars to value_buffer
		while(*input_ptr != '\0' && i < VALUE_BUFFER_MAX) {
			value_buffer[i] = *input_ptr;
			input_ptr++;
			i++;
		}

		//add endmark
		value_buffer[i] = '\0';

		//try to parse string in value_buffer to double
		// greater 0 - success; else failure
		if(sscanf(value_buffer, "%lf", &testval)) {

			//respect max
			if(*value >= max) {
				*value = max;
			}

			//respect min
			if(*value < min) {
				*value = min;
			}

			return true;
		}
	}
	//if strlen() mismatch or  parsing of sscanf() fails return false;
	return false;
}
*/
