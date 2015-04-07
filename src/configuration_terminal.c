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
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

/* * local headers               * */
#include "lib/uart.h"

#include "lib/twi_master.h"

#include "configuration_storage.h"
#include "motionsensor.h"
#include "l6205.h"
#include "vt100.h"

/* *** DEFINES ************************************************************** */
#define INPUT_BUFFER_MAX				40
#define INPUT_BUFFER_PARSE_MINLEN		3

#define INPUT_BUFFER_GROUP_INDEX		0
#define INPUT_BUFFER_COMMAND_INDEX		1
#define INPUT_BUFFER_SEPERATOR_INDEX	2
#define INPUT_BUFFER_SEPERATOR			','

#define VALUE_BUFFER_MAX				10

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
	STATE_SET_PRINT_DATA_MODE,
	STATE_RESET_CONFIG,
	STATE_SAVE_CONFIG,
	STATE_LIVE_DATA,
	STATE_FINAL,
	STATE_NULL
} configuration_terminal_state_t;

/* * local objects               * */
static configuration_terminal_state_t current_state;
static configuration_terminal_state_t next_state;

static char input_buffer[INPUT_BUFFER_MAX];

//string const
static const char string_terminal_center_pid_p[] PROGMEM = "pid_center.p: %i";
static const char string_terminal_center_pid_i[] PROGMEM = "pid_center.i: %i";
static const char string_terminal_center_pid_d[] PROGMEM = "pid_center.d: %i";
static const char string_terminal_center_pid_s[] PROGMEM = "pid_center.scalingfactor: %u";
static const char string_terminal_edge_pid_p[] PROGMEM = "pid_edge.p: %i";
static const char string_terminal_edge_pid_i[] PROGMEM = "pid_edge.i: %i";
static const char string_terminal_edge_pid_d[] PROGMEM = "pid_edge.d: %i";
static const char string_terminal_edge_pid_s[] PROGMEM = "pid_edge.scalingfactor: %u";
static const char string_terminal_edge_angle[] PROGMEM = "pid_edge_angle: %u";
static const char string_terminal_complementary_filter_ratio[] PROGMEM = "complementary_filter.ratio: %f";
static const char string_terminal_angle_scaling[] PROGMEM = "motionsensor.angle_scalingfactor: %u";
static const char string_terminal_motionsensor_offset_accel_x[] PROGMEM = "motionsensor.acceleration_offset.x: %i";
static const char string_terminal_motionsensor_offset_accel_y[] PROGMEM = "motionsensor.acceleration_offset.y: %i";
static const char string_terminal_motionsensor_offset_accel_z[] PROGMEM = "motionsensor.acceleration_offset.z: %i";
static const char string_terminal_motionsensor_offset_angular_x[] PROGMEM = "motionsensor.angularvelocity_offset.x: %i";
static const char string_terminal_motionsensor_offset_angular_y[] PROGMEM = "motionsensor.angularvelocity_offset.y: %i";
static const char string_terminal_motionsensor_offset_angular_z[] PROGMEM = "motionsensor.angularvelocity_offset.z: %i";
static const char string_terminal_motor_acceleration[] PROGMEM = "motor_control.acceleration: %i";
static const char string_terminal_print_data_mode[] PROGMEM = "print_data_mode: %i";
static const char string_terminal_help[] PROGMEM = "enter '?' for help";

static const char string_print_help_center_pid_p[] PROGMEM = "cp,<0..%d> - set proportional factor of center PID controller";
static const char string_print_help_center_pid_i[] PROGMEM = "ci,<0..%d> - set integral factor of center PID controller";
static const char string_print_help_center_pid_d[] PROGMEM = "cd,<0..%d> - set differential factor of center PID controller";
static const char string_print_help_center_pid_s[] PROGMEM = "cs,<0..%d> - set the result divider of the center PID controller";
static const char string_print_help_edge_pid_p[] PROGMEM = "ep,<0..%d> - set proportional factor of edge PID controller";
static const char string_print_help_edge_pid_i[] PROGMEM = "ei,<0..%d> - set integral factor of egde PID controller";
static const char string_print_help_edge_pid_d[] PROGMEM = "ed,<0..%d> - set differential factor of egde PID controller";
static const char string_print_help_edge_pid_s[] PROGMEM = "es,<0..%d> - set the result divider of the egde PID controller";
static const char string_print_help_edge_angle[] PROGMEM = "ea,<0..%d> - set edge angle at which egde PID controller should be active";
static const char string_print_help_complementary_filter_ratio[] PROGMEM = "fc,<0..%d> - set the ratio of complementary filter";
static const char string_print_help_angle_scaling[] PROGMEM = "fa,<0..%d> - set the angle scaling factor";
static const char string_print_help_motor_acceleration[] PROGMEM = "ma,<0..%d> - set the motor acceleration";
static const char string_print_help_print_data_mode_1[] PROGMEM = "pm,<0..%d> - set print data mode";
static const char string_print_help_print_data_mode_2[] PROGMEM = "    0 - none, 1 - ticker, 2 - angle & pid, 3 - all raw";
static const char string_print_help_print_data_mode_3[] PROGMEM = "    4 - all filtered, 5 - really all (filtered)";
static const char string_print_help_print_data_new_offsets[] PROGMEM = "z - set new offsets for motionsensor";
static const char string_print_help[] PROGMEM = "? - print this help";
static const char string_print_help_current_config[] PROGMEM = "s - show current configuration";
static const char string_print_help_reset_config[] PROGMEM = "r - reset configuration to defaults";
static const char string_print_help_save_and_quit[] PROGMEM = "x - quit terminal and DISCARD changes";
static const char string_print_help_discard_and_quit[] PROGMEM = "q - quit terminal and SAVE changes to eeprom";

static const char string_input[] PROGMEM = "> ";
static const char string_LF[] PROGMEM = "\n";
static const char string_OK[] PROGMEM = "OK\n";
static const char string_INVALID_SELECT[] PROGMEM = "invalid selection!\n";
static const char string_INVALID_NUMBER[] PROGMEM = "invalid number!\n";
//static const char string_ERROR[] PROGMEM = "error!\n";

/* * local function declarations * */
static void configuration_terminal_state_read_input(void);
static void configuration_terminal_state_print_help(void);
static void configuration_terminal_state_print_config(void);
static void configuration_terminal_state_set_pid_center_parameter(void);
static void configuration_terminal_state_set_pid_edge_parameter(void);
static void configuration_terminal_state_set_motor_parameter(void);
static void configuration_terminal_state_set_filter_parameter(void);
static void configuration_terminal_state_set_offset(void);
static void configuration_terminal_state_set_print_data_mode(void);
static void configuration_terminal_state_reset_configuration(void);
static void configuration_terminal_state_save_configuration(void);


static bool parse_input2int16(int16_t *value, const int16_t min, const int16_t max);
static bool parse_input2float(float *value, const float min, const float max);

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

			case STATE_SET_PRINT_DATA_MODE:
				configuration_terminal_state_set_print_data_mode();
			break;

			case STATE_RESET_CONFIG:
				configuration_terminal_state_reset_configuration();
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

void configuration_terminal_state_read_input(void)
{
	//ENTRY
	//DO

	vt100_clear_input_buffer();
	UART_char_received();

	printf_P(string_input);

	vt100_clear_input_buffer();
	vt100_get_string(input_buffer,INPUT_BUFFER_MAX);

	switch (input_buffer[INPUT_BUFFER_GROUP_INDEX]) {
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
		case 'p':
			next_state = STATE_SET_PRINT_DATA_MODE;
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
		case 'x':
			next_state = STATE_FINAL;
			break;
		default:
			next_state = STATE_READ_INPUT;
			break;
	}

	vt100_clear_input_buffer();
}


void configuration_terminal_state_print_help(void)
{
	printf_P(string_print_help_center_pid_p, INT16_MAX);		printf_P(string_LF);
	printf_P(string_print_help_center_pid_i, INT16_MAX);		printf_P(string_LF);
	printf_P(string_print_help_center_pid_d, INT16_MAX);		printf_P(string_LF);
	printf_P(string_print_help_center_pid_s, INT16_MAX);		printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help_edge_pid_p, INT16_MAX);			printf_P(string_LF);
	printf_P(string_print_help_edge_pid_i, INT16_MAX);			printf_P(string_LF);
	printf_P(string_print_help_edge_pid_d, INT16_MAX);			printf_P(string_LF);
	printf_P(string_print_help_edge_pid_s, INT16_MAX);			printf_P(string_LF);

	printf_P(string_print_help_edge_angle, INT16_MAX);			printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help_complementary_filter_ratio, MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE); printf_P(string_LF);
	printf_P(string_print_help_angle_scaling, INT16_MAX);												printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help_motor_acceleration, L6205_ACCELERATION_MAX);								printf_P(string_LF);
	printf_P(string_print_help_print_data_mode_1, FINAL_print_data_enum_t_ENTRY - 1);					printf_P(string_LF);
	printf_P(string_print_help_print_data_mode_2);														printf_P(string_LF);
	printf_P(string_print_help_print_data_mode_3);														printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help_print_data_new_offsets);			printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help);								printf_P(string_LF);
	printf_P(string_print_help_current_config);					printf_P(string_LF);
	printf_P(string_print_help_reset_config);					printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help_save_and_quit);					printf_P(string_LF);
	printf_P(string_print_help_discard_and_quit);				printf_P(string_LF); printf_P(string_LF);

	next_state = STATE_READ_INPUT;
}
void configuration_terminal_state_print_config(void)
{
	acceleration_vector_t accel_offset;
	configuration_storage_get_acceleration_offset_vector(&accel_offset);

	angularvelocity_vector_t angular_offset;
	configuration_storage_get_angularvelocity_offset_vector(&angular_offset);

	printf_P(string_terminal_center_pid_p, configuration_storage_get_pid_center_p_factor());				printf_P(string_LF);
	printf_P(string_terminal_center_pid_i, configuration_storage_get_pid_center_i_factor());				printf_P(string_LF);
	printf_P(string_terminal_center_pid_d, configuration_storage_get_pid_center_d_factor());				printf_P(string_LF);
	printf_P(string_terminal_center_pid_s, configuration_storage_get_pid_center_scalingfactor());			printf_P(string_LF);
	printf_P(string_terminal_edge_pid_p, configuration_storage_get_pid_edge_p_factor());					printf_P(string_LF);
	printf_P(string_terminal_edge_pid_i, configuration_storage_get_pid_edge_i_factor());					printf_P(string_LF);
	printf_P(string_terminal_edge_pid_d, configuration_storage_get_pid_edge_d_factor());					printf_P(string_LF);
	printf_P(string_terminal_edge_pid_s, configuration_storage_get_pid_edge_scalingfactor());				printf_P(string_LF);
	printf_P(string_terminal_edge_angle, configuration_storage_get_pid_edge_angle());						printf_P(string_LF);
	printf_P(string_terminal_complementary_filter_ratio, configuration_storage_get_complementary_filter_ratio());		printf_P(string_LF);
	printf_P(string_terminal_angle_scaling, configuration_storage_get_angle_scalingfactor());				printf_P(string_LF);
	printf_P(string_terminal_motionsensor_offset_accel_x, accel_offset.x);									printf_P(string_LF);
	printf_P(string_terminal_motionsensor_offset_accel_y, accel_offset.y);									printf_P(string_LF);
	printf_P(string_terminal_motionsensor_offset_accel_z, accel_offset.z);									printf_P(string_LF);
	printf_P(string_terminal_motionsensor_offset_angular_x, angular_offset.x);								printf_P(string_LF);
	printf_P(string_terminal_motionsensor_offset_angular_y, angular_offset.y);								printf_P(string_LF);
	printf_P(string_terminal_motionsensor_offset_angular_z, angular_offset.z);								printf_P(string_LF);
	printf_P(string_terminal_motor_acceleration, configuration_storage_get_motor_acceleration());			printf_P(string_LF);
	printf_P(string_terminal_print_data_mode, configuration_storage_get_print_data_mode());					printf_P(string_LF); 	printf_P(string_LF);
	printf_P(string_terminal_help);																			printf_P(string_LF);	printf_P(string_LF);

	next_state = STATE_READ_INPUT;
}



void configuration_terminal_state_set_pid_center_parameter(void)
{
	int16_t pid_value = 0;

	//parsing string to integer
	if(parse_input2int16(&pid_value, 0, INT16_MAX)) {

		switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
			case 'p':
				configuration_storage_set_pid_center_p_factor(pid_value);
				printf_P(string_OK);
				break;
			case 'i':
				configuration_storage_set_pid_center_i_factor(pid_value);
				printf_P(string_OK);
				break;
			case 'd':
				configuration_storage_set_pid_center_d_factor(pid_value);
				printf_P(string_OK);
				break;
			case 's':
				configuration_storage_set_pid_center_scalingfactor(pid_value);
				printf_P(string_OK);
				break;
			default:
				printf_P(string_INVALID_SELECT);
				break;
		}
	} else {
		printf_P(string_INVALID_NUMBER);
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_pid_edge_parameter(void)
{
	int16_t pid_value = 0;

	//parsing string to integer
	if(parse_input2int16(&pid_value, 0, INT16_MAX)) {

		switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
			case 'p':
				configuration_storage_set_pid_edge_p_factor(pid_value);
				printf_P(string_OK);
				break;
			case 'i':
				configuration_storage_set_pid_edge_i_factor(pid_value);
				printf_P(string_OK);
				break;
			case 'd':
				configuration_storage_set_pid_edge_d_factor(pid_value);
				printf_P(string_OK);
				break;
			case 's':
				configuration_storage_set_pid_edge_scalingfactor(pid_value);
				printf_P(string_OK);
				break;
			case 'a':
				configuration_storage_set_pid_edge_angle(pid_value);
				printf_P(string_OK);
				break;
			default:
				printf_P(string_INVALID_SELECT);
				break;
		}
	} else {
		printf_P(string_INVALID_NUMBER);
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_filter_parameter(void)
{
	int16_t tmp_int16 = 0;
	float tmp_float = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'c':
			//if(parse_input2int16(&tmp_int16, 0, MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE)) {
			if(parse_input2float(&tmp_float, 0, MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE)) {
				configuration_storage_set_complementary_filter_ratio(tmp_float);
				printf_P(string_OK);
			} else {
				printf_P(string_INVALID_NUMBER);
			}
			break;

		case 'a':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_angle_scalingfactor(tmp_int16);
				printf_P(string_OK);
			} else {
				printf_P(string_INVALID_NUMBER);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_motor_parameter(void)
{
	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'a':
			if(parse_input2int16(&tmp_int16, 0, L6205_ACCELERATION_MAX)) {
				configuration_storage_set_motor_acceleration((uint8_t)(tmp_int16));
				printf(string_OK);
			} else {
				printf_P(string_INVALID_NUMBER);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
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

void configuration_terminal_state_set_print_data_mode(void)
{
	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'm':
			if(parse_input2int16(&tmp_int16, 0, FINAL_print_data_enum_t_ENTRY - 1)) {
				configuration_storage_set_print_data_mode((uint8_t)(tmp_int16));
				printf_P(string_OK);
			} else {
				printf_P(string_INVALID_NUMBER);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

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

bool parse_input2int16(int16_t *value, const int16_t min, const int16_t max)
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



bool parse_input2float(float *value, const float min, const float max)
{
	char *input_ptr = input_buffer;		//set intput_ptr to begin of input_buffer
	char value_buffer[VALUE_BUFFER_MAX];
	uint8_t i = 0;

	//try parsing only string witch is long enough, has a seperator at the right index
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

		//try to convert to float
		*value = atof(value_buffer);

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
	return false;
}
