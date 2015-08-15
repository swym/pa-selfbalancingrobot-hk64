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
#include "uart.h"

#include "lib/twi_master.h"

#include "configuration_storage.h"
#include "motionsensor.h"
#include "l6205.h"
//#include "vt100.h"

/* *** DEFINES ************************************************************** */
#define INPUT_BUFFER_MAX				40
#define INPUT_BUFFER_PARSE_MINLEN		3

#define INPUT_BUFFER_GROUP_INDEX		0
#define INPUT_BUFFER_COMMAND_INDEX		1
#define INPUT_BUFFER_SEPERATOR_INDEX	2
#define INPUT_BUFFER_SEPERATOR			','

#define INPUT_BUFFER_PID_TYPE_INDEX			0		//selected pid reuses input buffer
#define INPUT_BUFFER_PID_PARAMETER_INDEX	1

#define VALUE_BUFFER_MAX				10

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
typedef enum {
	STATE_READ_INPUT,
	STATE_PRINT_HELP,
	STATE_PRINT_CONFIG,
	STATE_READ_PID_SELECTION,
	STATE_SET_PID,
	STATE_SET_FILTER_PARAMETER,
	STATE_SET_MOTOR_PARAMETER,
	STATE_SET_OFFSET_ACCELERATION,
	STATE_SET_OFFSET_ANGULARVELOCITY,
	STATE_SET_OFFSET_AUTOMATIC,
	STATE_SET_SPEED_STRAIGHT_LIMITS,
	STATE_SET_SPEED_TURN_LIMITS,
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

static const char string_terminal_name_pid_robot_position[] PROGMEM = "pid_robot_position:";
static const char string_terminal_name_pid_speed[]          PROGMEM = "pid_robot_speed:   ";
static const char string_terminal_name_pid_balance[]        PROGMEM = "pid_balance:       ";
static const char string_terminal_name_pid_motor_one[]      PROGMEM = "pid_motor_1:       ";
static const char string_terminal_name_pid_motor_two[]      PROGMEM = "pid_motor_2:       ";

static const char string_terminal_pid_header[] PROGMEM = "                         p      i      d      s      l";
//string const
static const char string_terminal_pid_p[] PROGMEM = " %6i";
static const char string_terminal_pid_i[] PROGMEM = " %6i";
static const char string_terminal_pid_d[] PROGMEM = " %6i";
static const char string_terminal_pid_s[] PROGMEM = " %6i";
static const char string_terminal_pid_l[] PROGMEM = " %6i";

static const char string_terminal_complementary_filter_ratio[] PROGMEM = "complementary_filter.ratio: %f";
static const char string_terminal_valid_acceleration_magnitude[] PROGMEM = "valid_acceleration_magnitude: %f";
static const char string_terminal_angle_scaling[] PROGMEM = "motionsensor.angle_scalingfactor: %u";

static const char string_terminal_speed_limits_straight[] PROGMEM = "max speed:  straight   limit: %4i   step: %4i";
static const char string_terminal_speed_limits_turn[] PROGMEM =     "max speed:  turn       limit: %4i   step: %4i";

static const char string_terminal_offset_header[] PROGMEM  = "                             x      y      z";
static const char string_terminal_offset_accel[] PROGMEM   = "acceleration_offset:   ";
static const char string_terminal_offset_angular[] PROGMEM = "angularvelocity_offset:";
static const char string_terminal_offset_xyz[] PROGMEM = " %6i %6i %6i";

static const char string_terminal_motor_acceleration[] PROGMEM = "motor_control.acceleration: %i";
static const char string_terminal_print_data_mode[] PROGMEM = "print_data_mode: %i";
static const char string_terminal_help[] PROGMEM = "enter '?' for help";

// ---
static const char string_print_help_pid_1[] PROGMEM = "P - for enter PID mode; Then enter:";
static const char string_print_help_pid_2[] PROGMEM = "PV,<0..max> - where P is pid identifier and V value identifier. \n V corresponds to value table \n Current pid ids:";
static const char string_print_help_pid_3[] PROGMEM = "          r - robot_position";
static const char string_print_help_pid_4[] PROGMEM = "          s - robot_speed";
static const char string_print_help_pid_5[] PROGMEM = "          b - balance";
static const char string_print_help_pid_6[] PROGMEM = "          1 - motor_1";
static const char string_print_help_pid_7[] PROGMEM = "          2 - motor_2";

static const char string_print_offset_acceleration[] PROGMEM = "a?,<0..max> - manual offset for acceleration where '?' is axis";
static const char string_print_offset_angularvelocity[] PROGMEM = "v?,<0..max> - manual offset for angularvelocity where '?' is axis";

static const char string_print_limits_1[] PROGMEM = "sl,<0..max> - speed straight limit";
static const char string_print_limits_2[] PROGMEM = "ss,<0..max> - speed straight step";
static const char string_print_limits_3[] PROGMEM = "tl,<0..max> - speed turn limit";
static const char string_print_limits_4[] PROGMEM = "ts,<0..max> - speed turn step";

static const char string_print_help_complementary_filter_ratio[] PROGMEM = "fc,<0..%f> - set the ratio of complementary filter";
static const char string_print_help_valid_acceleration_magnitude[] PROGMEM = "fm,<0..%f> - set validacceleration magnitude";
static const char string_print_help_angle_scaling[] PROGMEM = "fa,<0..%d> - set the angle scaling factor";

static const char string_print_help_motor_acceleration[] PROGMEM = "ma,<0..%d> - set the motor acceleration";

static const char string_print_help_print_data_mode_1[] PROGMEM = "pm,<0..%d> - set print data mode";
static const char string_print_help_print_data_mode_2[] PROGMEM = "    0 - none, 1 - ticker, 2 - angle & pid, 3 - all raw";
static const char string_print_help_print_data_mode_3[] PROGMEM = "    4 - all filtered, 5 - really all (filtered)";
static const char string_print_help_print_data_new_offsets[] PROGMEM = "z - set new offsets for motionsensor";
static const char string_print_help[] PROGMEM = "? - print this help";
static const char string_print_help_current_config[] PROGMEM = "S - show current configuration";
static const char string_print_help_reset_config[] PROGMEM = "R - reset configuration to defaults";
static const char string_print_help_save_and_quit[] PROGMEM = "X - quit terminal and DISCARD changes";
static const char string_print_help_discard_and_quit[] PROGMEM = "Q - quit terminal and SAVE changes to eeprom";

static const char string_input[] PROGMEM = "> ";
static const char string_LF[] PROGMEM = "\n";
static const char string_OK[] PROGMEM = "OK\n";
static const char string_INVALID_SELECT[] PROGMEM = "invalid selection!\n";
static const char string_INVALID_NUMBER[] PROGMEM = "invalid number!\n";
//static const char string_ERROR[] PROGMEM = "error!\n";

/* * local function declarations * */
static void configuration_terminal_state_read_input(void);
static void configuration_terminal_state_read_pid_selection_pid(void);
static void configuration_terminal_state_print_help(void);
static void configuration_terminal_state_print_config(void);
static void configuration_terminal_state_set_pid_parameter(void);
static void configuration_terminal_state_set_motor_parameter(void);
static void configuration_terminal_state_set_filter_parameter(void);
static void configuration_terminal_state_set_offset_acceleration(void);
static void configuration_terminal_state_set_offset_angularvelocity(void);
static void configuration_terminal_state_set_offset_automatic(void);
static void configuration_terminal_state_set_speed_straight_limits(void);
static void configuration_terminal_state_set_speed_turn_limits(void);
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

			case STATE_READ_PID_SELECTION:
				configuration_terminal_state_read_pid_selection_pid();
				break;

			case STATE_PRINT_HELP:
				configuration_terminal_state_print_help();
				break;

			case STATE_PRINT_CONFIG:
				configuration_terminal_state_print_config();
				break;

			case STATE_SET_PID:
				configuration_terminal_state_set_pid_parameter();
				break;

			case STATE_SET_FILTER_PARAMETER:
				configuration_terminal_state_set_filter_parameter();
				break;

			case STATE_SET_MOTOR_PARAMETER:
				configuration_terminal_state_set_motor_parameter();
				break;

			case STATE_SET_OFFSET_ACCELERATION:
				configuration_terminal_state_set_offset_acceleration();
				break;

			case STATE_SET_OFFSET_ANGULARVELOCITY:
				configuration_terminal_state_set_offset_angularvelocity();
				break;

			case STATE_SET_OFFSET_AUTOMATIC:
				configuration_terminal_state_set_offset_automatic();
				break;

			case STATE_SET_SPEED_STRAIGHT_LIMITS:
				configuration_terminal_state_set_speed_straight_limits();
				break;

			case STATE_SET_SPEED_TURN_LIMITS:
				configuration_terminal_state_set_speed_turn_limits();
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

	// get user input
	uart_flush();
	printf_P(string_input);

	while(uart_gets(input_buffer, INPUT_BUFFER_MAX) == 0) {
		//wait for completed string
	}
	uart_flush();

	switch (input_buffer[INPUT_BUFFER_GROUP_INDEX]) {
		case 'P':
			next_state = STATE_READ_PID_SELECTION;
			break;
		case 'f':
			next_state = STATE_SET_FILTER_PARAMETER;
			break;
		case 'm':
			next_state = STATE_SET_MOTOR_PARAMETER;
			break;
		case 'a':
			next_state = STATE_SET_OFFSET_ACCELERATION;
			break;
		case 'v':
			next_state = STATE_SET_OFFSET_ANGULARVELOCITY;
			break;
		case 'z':
			next_state = STATE_SET_OFFSET_AUTOMATIC;
			break;
		case 's':
			next_state = STATE_SET_SPEED_STRAIGHT_LIMITS;
			break;
		case 't':
			next_state = STATE_SET_SPEED_TURN_LIMITS;
			break;
		case 'd':
			next_state = STATE_SET_PRINT_DATA_MODE;
			break;
		case '?':
			next_state = STATE_PRINT_HELP;
			break;
		case 'S':
			next_state = STATE_PRINT_CONFIG;
			break;
		case 'R':
			next_state = STATE_RESET_CONFIG;
			break;
		case 'Q':
			next_state = STATE_SAVE_CONFIG;
			break;
		case 'X':
			next_state = STATE_FINAL;
			break;
		default:
			next_state = STATE_READ_INPUT;
			break;
	}
}

//This state reuses input buffer to have two input bytes again for deciding selected parameter and value
void configuration_terminal_state_read_pid_selection_pid(void)
{
	// get user input
	uart_flush();

	printf_P(string_LF);
	printf("PID: >");

	while(uart_gets(input_buffer, INPUT_BUFFER_MAX) == 0) {
		//wait for completed string
	}
	uart_flush();

	next_state = STATE_SET_PID;
}



void configuration_terminal_state_print_help(void)
{
	printf_P(string_print_help_pid_1);	printf_P(string_LF);
	printf_P(string_print_help_pid_2);	printf_P(string_LF);
	printf_P(string_print_help_pid_3);	printf_P(string_LF);
	printf_P(string_print_help_pid_4);	printf_P(string_LF);
	printf_P(string_print_help_pid_5);	printf_P(string_LF);
	printf_P(string_print_help_pid_6);	printf_P(string_LF);
	printf_P(string_print_help_pid_7);	printf_P(string_LF);
	printf_P(string_LF);

	printf_P(string_print_offset_acceleration);		printf_P(string_LF);
	printf_P(string_print_offset_angularvelocity);	printf_P(string_LF);

	printf_P(string_print_help_complementary_filter_ratio, MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE); printf_P(string_LF);
	printf_P(string_print_help_valid_acceleration_magnitude, 1.0);										printf_P(string_LF);
	printf_P(string_print_help_angle_scaling, INT16_MAX);												printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_help_motor_acceleration, L6205_ACCELERATION_MAX);								printf_P(string_LF);
	printf_P(string_print_help_print_data_mode_1, FINAL_print_data_enum_t_ENTRY - 1);					printf_P(string_LF);
	printf_P(string_print_help_print_data_mode_2);														printf_P(string_LF);
	printf_P(string_print_help_print_data_mode_3);														printf_P(string_LF); printf_P(string_LF);

	printf_P(string_print_limits_1);	printf_P(string_LF);
	printf_P(string_print_limits_2);	printf_P(string_LF);
	printf_P(string_print_limits_3);	printf_P(string_LF);
	printf_P(string_print_limits_4);	printf_P(string_LF);

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


	printf_P(string_terminal_pid_header);
	printf_P(string_LF);

	printf_P(string_terminal_name_pid_robot_position);
	printf_P(string_terminal_pid_p, configuration_storage_get_pid_p_factor(PID_ROBOT_POSITION));
	printf_P(string_terminal_pid_i, configuration_storage_get_pid_i_factor(PID_ROBOT_POSITION));
	printf_P(string_terminal_pid_d, configuration_storage_get_pid_d_factor(PID_ROBOT_POSITION));
	printf_P(string_terminal_pid_s, configuration_storage_get_pid_scalingfactor(PID_ROBOT_POSITION));
	printf_P(string_terminal_pid_l, configuration_storage_get_pid_limit(PID_ROBOT_POSITION));
	printf_P(string_LF);

	printf_P(string_terminal_name_pid_speed);
	printf_P(string_terminal_pid_p, configuration_storage_get_pid_p_factor(PID_ROBOT_SPEED));
	printf_P(string_terminal_pid_i, configuration_storage_get_pid_i_factor(PID_ROBOT_SPEED));
	printf_P(string_terminal_pid_d, configuration_storage_get_pid_d_factor(PID_ROBOT_SPEED));
	printf_P(string_terminal_pid_s, configuration_storage_get_pid_scalingfactor(PID_ROBOT_SPEED));
	printf_P(string_terminal_pid_l, configuration_storage_get_pid_limit(PID_ROBOT_SPEED));
	printf_P(string_LF);

	printf_P(string_terminal_name_pid_balance);
	printf_P(string_terminal_pid_p, configuration_storage_get_pid_p_factor(PID_BALANCE));
	printf_P(string_terminal_pid_i, configuration_storage_get_pid_i_factor(PID_BALANCE));
	printf_P(string_terminal_pid_d, configuration_storage_get_pid_d_factor(PID_BALANCE));
	printf_P(string_terminal_pid_s, configuration_storage_get_pid_scalingfactor(PID_BALANCE));
	printf_P(string_terminal_pid_l, configuration_storage_get_pid_limit(PID_BALANCE));
	printf_P(string_LF);

	printf_P(string_terminal_name_pid_motor_one);
	printf_P(string_terminal_pid_p, configuration_storage_get_pid_p_factor(PID_MOTOR_1));
	printf_P(string_terminal_pid_i, configuration_storage_get_pid_i_factor(PID_MOTOR_1));
	printf_P(string_terminal_pid_d, configuration_storage_get_pid_d_factor(PID_MOTOR_1));
	printf_P(string_terminal_pid_s, configuration_storage_get_pid_scalingfactor(PID_MOTOR_1));
	printf_P(string_terminal_pid_l, configuration_storage_get_pid_limit(PID_MOTOR_1));
	printf_P(string_LF);

	printf_P(string_terminal_name_pid_motor_two);
	printf_P(string_terminal_pid_p, configuration_storage_get_pid_p_factor(PID_MOTOR_2));
	printf_P(string_terminal_pid_i, configuration_storage_get_pid_i_factor(PID_MOTOR_2));
	printf_P(string_terminal_pid_d, configuration_storage_get_pid_d_factor(PID_MOTOR_2));
	printf_P(string_terminal_pid_s, configuration_storage_get_pid_scalingfactor(PID_MOTOR_2));
	printf_P(string_terminal_pid_l, configuration_storage_get_pid_limit(PID_MOTOR_2));
	printf_P(string_LF);
	printf_P(string_LF);

	printf_P(string_terminal_offset_header);
	printf_P(string_LF);

	printf_P(string_terminal_offset_accel);
	printf_P(string_terminal_offset_xyz, accel_offset.x, accel_offset.y, accel_offset.z);
	printf_P(string_LF);

	printf_P(string_terminal_offset_angular);
	printf_P(string_terminal_offset_xyz, angular_offset.x, angular_offset.y, angular_offset.z);
	printf_P(string_LF);
	printf_P(string_LF);

	printf_P(string_terminal_complementary_filter_ratio, configuration_storage_get_complementary_filter_ratio());		printf_P(string_LF);
	printf_P(string_terminal_valid_acceleration_magnitude, configuration_storage_get_valid_acceleration_magnitude());	printf_P(string_LF);
	printf_P(string_terminal_angle_scaling, configuration_storage_get_angle_scalingfactor());				printf_P(string_LF);



	printf_P(string_terminal_speed_limits_straight,
			 configuration_storage_get_speed_straight_limit(),
			 configuration_storage_get_speed_straight_step()); printf_P(string_LF);

	printf_P(string_terminal_speed_limits_turn,
			 configuration_storage_get_speed_turn_limit(),
			 configuration_storage_get_speed_turn_step()); printf_P(string_LF);

	printf_P(string_terminal_motor_acceleration, configuration_storage_get_motor_acceleration());			printf_P(string_LF);
	printf_P(string_terminal_print_data_mode, configuration_storage_get_print_data_mode());					printf_P(string_LF); 	printf_P(string_LF);
	printf_P(string_terminal_help);																			printf_P(string_LF);	printf_P(string_LF);

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_pid_parameter(void)
{
	int16_t value = 0;
	bool value_parsed = parse_input2int16(&value, 0, INT16_MAX);
	pid_types_enum_t selected_pid = PID_NONE;

	if(value_parsed) {
		switch (input_buffer[INPUT_BUFFER_PID_TYPE_INDEX]) {
			case'r':
				selected_pid = PID_ROBOT_POSITION;
				break;
			case's':
				selected_pid = PID_ROBOT_SPEED;
				break;
			case'b':
				selected_pid = PID_BALANCE;
				break;
			case'1':
				selected_pid = PID_MOTOR_1;
				break;
			case'2':
				selected_pid = PID_MOTOR_2;
				break;
			default:
				selected_pid = PID_NONE;
				break;
		}

		if(selected_pid != PID_NONE) {
			switch (input_buffer[INPUT_BUFFER_PID_PARAMETER_INDEX]) {
				case 'p':
					configuration_storage_set_pid_p_factor(selected_pid, value);
					break;
				case 'i':
					configuration_storage_set_pid_i_factor(selected_pid, value);
					break;
				case 'd':
					configuration_storage_set_pid_d_factor(selected_pid, value);
					break;
				case 's':
					configuration_storage_set_pid_scalingfactor(selected_pid, value);
					break;
				case 'l':
					configuration_storage_set_pid_limit(selected_pid, value);
					break;
				default:
					break;
			}

			printf_P(string_OK);
		}
		else {
			printf_P(string_INVALID_SELECT);
		}
	} else {
		printf_P(string_INVALID_SELECT);
	}

	next_state = STATE_READ_INPUT;
}


// command: 'f'
void configuration_terminal_state_set_filter_parameter(void)
{
	int16_t tmp_int16 = 0;
	float tmp_float = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'a':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_angle_scalingfactor(tmp_int16);
				printf_P(string_OK);
			} else {
				printf("fa,%d\n",configuration_storage_get_angle_scalingfactor());
			}
			break;

		case 'c':
			if(parse_input2float(&tmp_float, 0, MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE)) {
				configuration_storage_set_complementary_filter_ratio(tmp_float);
				printf_P(string_OK);
			} else {
				printf("fc,%f\n",configuration_storage_get_complementary_filter_ratio());
			}
			break;

		case 'm':
			if(parse_input2float(&tmp_float, 0, 1.0)) {
				configuration_storage_set_valid_acceleration_magnitude(tmp_float);
				printf_P(string_OK);
			} else {
				printf("fm,%f\n",configuration_storage_get_valid_acceleration_magnitude());
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
				printf("ma,%u\n",configuration_storage_get_motor_acceleration());
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_offset_acceleration(void)
{

	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'x':
			if(parse_input2int16(&tmp_int16, INT16_MIN, INT16_MAX)) {
				configuration_storage_set_acceleration_offset_value(AXIS_X, tmp_int16);
				printf_P(string_OK);
			}
			break;

		case 'y':
			if(parse_input2int16(&tmp_int16, INT16_MIN, INT16_MAX)) {
				configuration_storage_set_acceleration_offset_value(AXIS_Y, tmp_int16);
				printf_P(string_OK);
			}
			break;

		case 'z':
			if(parse_input2int16(&tmp_int16, INT16_MIN, INT16_MAX)) {
				configuration_storage_set_acceleration_offset_value(AXIS_Z, tmp_int16);
				printf_P(string_OK);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_offset_angularvelocity(void)
{

	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'x':
			if(parse_input2int16(&tmp_int16, INT16_MIN, INT16_MAX)) {
				configuration_storage_set_angularvelocity_offset_value(AXIS_X, tmp_int16);
				printf_P(string_OK);
			}
			break;

		case 'y':
			if(parse_input2int16(&tmp_int16, INT16_MIN, INT16_MAX)) {
				configuration_storage_set_angularvelocity_offset_value(AXIS_Y, tmp_int16);
				printf_P(string_OK);
			}
			break;

		case 'z':
			if(parse_input2int16(&tmp_int16, INT16_MIN, INT16_MAX)) {
				configuration_storage_set_angularvelocity_offset_value(AXIS_Z, tmp_int16);
				printf_P(string_OK);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_offset_automatic(void)
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

void configuration_terminal_state_set_speed_straight_limits(void)
{

	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'l':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_speed_straight_limit(tmp_int16);
				printf_P(string_OK);
			}
			break;

		case 's':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_speed_straight_step(tmp_int16);
				printf_P(string_OK);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_set_speed_turn_limits(void)
{
	int16_t tmp_int16 = 0;

	switch (input_buffer[INPUT_BUFFER_COMMAND_INDEX]) {
		case 'l':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_speed_turn_limit(tmp_int16);
				printf_P(string_OK);
			}
			break;

		case 's':
			if(parse_input2int16(&tmp_int16, 0, INT16_MAX)) {
				configuration_storage_set_speed_turn_step(tmp_int16);
				printf_P(string_OK);
			}
			break;

		default:
			printf_P(string_INVALID_SELECT);
			break;
	}

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
	printf(string_OK);
	next_state = STATE_READ_INPUT;
}

void configuration_terminal_state_save_configuration(void)
{
	configuration_storage_save_configuration();
	printf(string_OK);
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
		if(sscanf(value_buffer, "%i", value)) {

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
