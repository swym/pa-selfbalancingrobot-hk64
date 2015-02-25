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
#include <avr/pgmspace.h>
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
	STATE_SET_PID_CENTER_PARAMETER,
	STATE_SET_PID_EDGE_PARAMETER,
	STATE_SET_FILTER_PARAMETER,
	STATE_SET_MOTOR_PARAMETER,
	STATE_SET_OFFSET,
	STATE_RESET_CONFIG,
	STATE_SAVE_CONFIG,
	STATE_LIVE_DATA,
	STATE_RUN_DEBUG,
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
static void configuration_terminal_state_set_pid_center_parameter(void);
static void configuration_terminal_state_set_pid_edge_parameter(void);
static void configuration_terminal_state_set_motor_parameter(void);
static void configuration_terminal_state_set_filter_parameter(void);
static void configuration_terminal_state_set_offset(void);
static void configuration_terminal_state_reset_configuration(void);
static void configuration_terminal_state_save_configuration(void);
static void configuration_terminal_state_run_debug(void);


static bool parse_input2int16(int16_t *value, int16_t min, int16_t max);

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

			case STATE_SET_PID_CENTER_PARAMETER:
				configuration_terminal_state_set_pid_center_parameter();
			break;

			case STATE_SET_PID_EDGE_PARAMETER:
				configuration_terminal_state_set_pid_edge_parameter();
			break;

			case STATE_SET_FILTER_PARAMETER:
				configuration_terminal_state_set_filter_parameter();
			break;

			case STATE_SET_MOTOR_PARAMETER:
				configuration_terminal_state_set_motor_parameter();
			break;

			case STATE_SET_OFFSET:
				configuration_terminal_state_set_offset();
			break;

			case STATE_RESET_CONFIG:
				configuration_terminal_state_reset_configuration();
			break;

			case STATE_SAVE_CONFIG:
				configuration_terminal_state_save_configuration();
			break;

			case STATE_RUN_DEBUG:
				configuration_terminal_state_run_debug();
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
	printf("cp,<0..%d> - set proportional factor of center PID controller\n", INT16_MAX);
	printf("ci,<0..%d> - set integral factor of center PID controller\n", INT16_MAX);
	printf("cd,<0..%d> - set differential factor of center PID controller\n", INT16_MAX);
	printf("ps,<0..%d> - set the result divider of the center PID controller\n\n", INT16_MAX);

	printf("ep,<0..%d> - set proportional factor of edge PID controller\n", INT16_MAX);
	printf("ei,<0..%d> - set integral factor of egde PID controller\n", INT16_MAX);
	printf("ed,<0..%d> - set differential factor of egde PID controller\n", INT16_MAX);
	printf("es,<0..%d> - set the result divider of the egde PID controller\n\n", INT16_MAX);

	printf("ea,<0..%d> - set edge angle at which egde PID controller should be active\n\n", UINT16_MAX - 1);

	printf("fc,<0..%d> - set the ratio of complementary filter\n", 100);
	printf("fa,<0..%d>  - set the angle scaling divider\n\n", INT16_MAX);

	printf("ma,<0..%d>  - set the angle scaling divider\n\n", 50);

	printf("z - set new offsets for motionsensor\n\n");



	printf("? - print this help\n");
	printf("s - show current configuration\n\n");

	printf("r - reset parameters to defaults\n");
	printf("x - quit terminal and DISCARD changes\n");
	printf("q - quit terminal and SAVE changes to eeprom\n");
	printf("d - quit terminal and SAVE changes and run in debug mode\n");


	next_state = STATE_READ_INPUT;
}
void configuration_terminal_state_print_config(void)
{
	acceleration_vector_t accel_offset;
	configuration_storage_get_acceleration_offset_vector(&accel_offset);

	angularvelocity_vector_t angular_offset;
	configuration_storage_get_angularvelocity_offset_vector(&angular_offset);

	printf("pid_center.p: %i\n",configuration_storage_get_pid_center_p_factor());
	printf("pid_center.i: %i\n",configuration_storage_get_pid_center_i_factor());
	printf("pid_center.d: %i\n",configuration_storage_get_pid_center_d_factor());
	printf("pid_center.scalingfactor: %u\n",configuration_storage_get_pid_center_scalingfactor());
	printf("pid_edge.p: %i\n",configuration_storage_get_pid_edge_p_factor());
	printf("pid_edge.i: %i\n",configuration_storage_get_pid_edge_i_factor());
	printf("pid_edge.d: %i\n",configuration_storage_get_pid_edge_d_factor());
	printf("pid_edge.scalingfactor: %u\n",configuration_storage_get_pid_edge_scalingfactor());
	printf("pid_edge_angle: %u\n",configuration_storage_get_pid_edge_angle());
	printf("complementary_filter.ratio (angularvelocity): %i\n",configuration_storage_get_complementary_filter_ratio());
	printf("complementary_filter.ratio (acceleration): %i\n", 100 - configuration_storage_get_complementary_filter_ratio());
	printf("motionsensor.angle_scalingfactor: %u\n",configuration_storage_get_angle_scalingfactor());
	printf("motionsensor.acceleration_offset.x: %i\n",accel_offset.x);
	printf("motionsensor.acceleration_offset.y: %i\n",accel_offset.y);
	printf("motionsensor.acceleration_offset.z: %i\n",accel_offset.z);
	printf("motionsensor.angularvelocity_offset.x: %i\n",angular_offset.x);
	printf("motionsensor.angularvelocity_offset.y: %i\n",angular_offset.y);
	printf("motionsensor.angularvelocity_offset.z: %i\n",angular_offset.z);
	printf("motor_control.acceleration: %i\n",configuration_storage_get_motor_acceleration());
	printf("\nenter '?' for help\n");

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

	switch (input_buffer[INPUT_BUFFER_COMMAND]) {
		case 'c':
			next_state = STATE_SET_PID_CENTER_PARAMETER;
			break;
		case 'e':
			next_state = STATE_SET_PID_EDGE_PARAMETER;
			break;
		case 'f':
			next_state = STATE_SET_FILTER_PARAMETER;
			break;
		case 'm':
			next_state = STATE_SET_MOTOR_PARAMETER;
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
		case 'r':
			next_state = STATE_RESET_CONFIG;
			break;
		case 'q':
			next_state = STATE_SAVE_CONFIG;
			break;
		case 'd':
			next_state = STATE_RUN_DEBUG;
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

void configuration_terminal_state_set_pid_center_parameter(void)
{
	int16_t pid_value = 0;

	//parsing string to integer
	if(parse_input2int16(&pid_value, 0, INT16_MAX)) {

		switch (input_buffer[INPUT_BUFFER_VALUE]) {
			case 'p':
				configuration_storage_set_pid_center_p_factor(pid_value);
				printf("OK\n");
				break;
			case 'i':
				configuration_storage_set_pid_center_i_factor(pid_value);
				printf("OK\n");
				break;
			case 'd':
				configuration_storage_set_pid_center_d_factor(pid_value);
				printf("OK\n");
				break;
			case 's':
				configuration_storage_set_pid_center_scalingfactor(pid_value);
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

void configuration_terminal_state_set_pid_edge_parameter(void)
{
	int16_t pid_value = 0;

	//parsing string to integer
	if(parse_input2int16(&pid_value, 0, INT16_MAX)) {

		switch (input_buffer[INPUT_BUFFER_VALUE]) {
			case 'p':
				configuration_storage_set_pid_edge_p_factor(pid_value);
				printf("OK\n");
				break;
			case 'i':
				configuration_storage_set_pid_edge_i_factor(pid_value);
				printf("OK\n");
				break;
			case 'd':
				configuration_storage_set_pid_edge_d_factor(pid_value);
				printf("OK\n");
				break;
			case 's':
				configuration_storage_set_pid_edge_scalingfactor(pid_value);
				printf("OK\n");
				break;
			case 'a':
				configuration_storage_set_pid_edge_angle(pid_value);
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

void configuration_terminal_state_set_filter_parameter(void)
{
	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_VALUE]) {
		case 'c':
			if(parse_input2int16(&tmp_int16, 0, 100)) {
				configuration_storage_set_complementary_filter_ratio(tmp_int16);
				printf("OK\n");
			} else {
				printf("invalid number\n");
			}
			break;

		case 'a':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_angle_scalingfactor(tmp_int16);
				printf("OK\n");
			} else {
				printf("invalid number\n");
			}
			break;

		default:
			printf("invalid value\n");
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_motor_parameter(void)
{
	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_VALUE]) {
		case 'a':
			if(parse_input2int16(&tmp_int16, 0, 50)) {
				configuration_storage_set_motor_acceleration((uint8_t)(tmp_int16));
				printf("OK\n");
			} else {
				printf("invalid number\n");
			}
			break;

		default:
			printf("invalid value\n");
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_offset(void)
{
	acceleration_vector_t acceleration_offset;
	angularvelocity_vector_t angularvelocity_offset;
	uint8_t rounds;

	//Perform calibration

	printf("will start calibration in 2 secs. ...\n");
	_delay_ms(2000);
	printf("start calibration...\n");

	for(rounds = 5; rounds > 0; rounds--) {
		if(rounds > 1) {
			printf("%d rounds to go...\n", rounds);
		} else {
			printf("%d round to go...\n", rounds);
		}

		motionsensor_calibrate_zero_point();
	}

	//Read and save the offset from the sensor
	motionsensor_get_acceleration_offset_vector(&acceleration_offset);
	configuration_storage_set_acceleration_offset_vector(&acceleration_offset);

	motionsensor_get_angularvelocity_offset_vector(&angularvelocity_offset);
	configuration_storage_set_angularvelocity_offset_vector(&angularvelocity_offset);

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_reset_configuration(void)
{
	configuration_storage_reset_configuration();
	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_save_configuration(void)
{
	configuration_storage_save_configuration();
	next_state = STATE_FINAL;
}

void configuration_terminal_state_run_debug(void)
{
	configuration_storage_save_configuration();
	configuration_storage_set_run_mode(CONFIGURATION_STORAGE_RUN_MODE_DEBUG);
	next_state = STATE_FINAL;
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
	//if strlen() mismatch or parsing of sscanf() fails return false;
	return false;
}
