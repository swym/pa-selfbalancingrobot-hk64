/*
 * system_controller.c
 *
 *  Created on: Dec 4, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "system_controller.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <avr/io.h>

#include <util/delay.h>

#include "uart.h"
#include "lib/twi_master.h"

#include "configuration_terminal.h"
#include "configuration_storage.h"
//#include "vt100.h"

#include "motionsensor.h"
#include "motor_control.h"
#include "pid.h"
#include "timer.h"
#include "base64.h"
#include "leds.h"

#include "filter.h"


/* *** DECLARATIONS ********************************************************** */


/* global types and constants */
// Timer
volatile timer_slot_t timer_current_majorslot;
volatile timer_slot_t timer_current_minorslot;

volatile uint16_t timer_command_timeout;

/* local type and constants     */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4

#define PRINT_DATA_BUFFER_SIZE 	20

#define COMMAND_BUFFER_MAX	20
#define COMMAND_TIMEOUT		5000

#define PID_ROBOT_POS_OUTPUT_MAX 16
#define PID_ROBOT_POS_OUTPUT_MIN (-PID_ROBOT_POS_OUTPUT_MAX)

#define PID_ROBOT_SLOWING_OUTPUT_MAX 24
#define PID_ROBOT_SLOWING_OUTPUT_MIN (-PID_ROBOT_SLOWING_OUTPUT_MAX)


typedef enum {
	STATE_INIT_BASIC_HARDWARE,
	STATE_LOAD_CONFIGURATON,
	STATE_WAITING_FOR_USER_INTERRUPT,
	STATE_RUN_CONFIGURATION_TERMINAL,
	STATE_INIT_CONTROLLER_ENVIRONMENT,
	STATE_INIT_REMAINING_HARDWARE,
	STATE_RUN_CONTROLLER,
	STATE_FINAL,
	STATE_NULL
} system_controller_state_t;

/* * local objects               * */
static system_controller_state_t current_state;
static system_controller_state_t next_state;

static motionsensor_angle_t current_angle;

static motor_control_speed_t current_speed;
static motor_control_speed_t new_motor_speed;

static pid_config_t				pid_robot_pos_config;
static pid_config_t				pid_balance_config;
static pid_config_t				pid_speed_motor_config;

static motionsensor_angle_t		angle_stable;

static pidData_t pid_robot_pos_data;
static pidData_t pid_robot_speed_data;
static pidData_t pid_balance_data;
static pidData_t pid_speed_m1_data;
static pidData_t pid_speed_m2_data;
static pidData_t pid_steering_data;

static int16_t	 pid_robot_pos_setpoint;
static int16_t	 pid_robot_speed_setpoint;
static int16_t   pid_balance_setpoint;
static int16_t	 pid_speed_m1_setpoint;
static int16_t	 pid_speed_m2_setpoint;

static int16_t	pid_robot_pos_output;		//angle setpoint
static int16_t	pid_robot_speed_output;
static int16_t	pid_balance_output;			//speed for motors
static int16_t	pid_speed_m1_output;		//pwm for drivers
static int16_t	pid_speed_m2_output;		//pwm for drivers

static int16_t	turning_offset;

static print_data_enum_t print_mode;
static bool print_data_enabled;
static void (*print_data_fptr)(void);
static uint8_t print_ticker_cnt;

static uint8_t print_data_buffer[PRINT_DATA_BUFFER_SIZE];

/* local function declarations  */

// states
static void system_controller_state_init_basic_hardware(void);
static void system_controller_state_load_configuration(void);
static void system_controller_state_waiting_for_user_interrupt(void);
static void system_controller_state_run_configuration_terminal(void);
static void system_controller_state_init_controller_environment(void);
static void system_controller_state_init_remaining_hardware(void);
static void system_controller_state_run_controller(void);

static void system_controller_halt_motors(void);

// print functions
static void system_controller_print_ticker(void);
static void system_controller_print_data_anglepid(void);
static void system_controller_print_data_all_raw(void);
static void system_controller_print_data_all_filtered(void);
static void system_controller_print_data_really_all_filtered(void);

//parse command
static void system_controller_parse_command(void);


/* *** FUNCTION DEFINITIONS ************************************************** */
void system_controller_state_machine(void)
{
	current_state = STATE_INIT_BASIC_HARDWARE;
	next_state = STATE_NULL;

	while(current_state != STATE_FINAL) {
		switch(current_state) {

			case STATE_INIT_BASIC_HARDWARE:
				system_controller_state_init_basic_hardware();
			break;

			case STATE_LOAD_CONFIGURATON:
				system_controller_state_load_configuration();
			break;

			case STATE_WAITING_FOR_USER_INTERRUPT:
				system_controller_state_waiting_for_user_interrupt();
			break;

			case STATE_RUN_CONFIGURATION_TERMINAL:
				system_controller_state_run_configuration_terminal();
			break;

			case STATE_INIT_CONTROLLER_ENVIRONMENT:
				system_controller_state_init_controller_environment();
			break;

			case STATE_INIT_REMAINING_HARDWARE:
				system_controller_state_init_remaining_hardware();
			break;

			case STATE_RUN_CONTROLLER:
				system_controller_state_run_controller();
			break;

			default:
				current_state = STATE_FINAL;
			break;

		}

		//change state
		current_state = next_state;
	}
}

void system_controller_state_init_basic_hardware(void)
{
	/* *** ENTRY *** */


	/* **** DO ***** */
	//printf("init LED Port...\n");
	leds_init();
	leds_set(1);


	uart_init(UART_BAUDRATE_9600);			/* Init USART */
	uart_init_stdio();
	//printf("usart inited\n");



	//printf("init TWI interface...\n");
	twi_master_init(TWI_TWBR_VALUE_400);	/* Init TWI/I2C Schnittstelle */

	//printf("enable interrupts...\n");
	sei();

	//init motionsensor
	printf("init motionsensor...\n");
	motionsensor_init();

	/* *** EXIT **** */

	next_state = STATE_LOAD_CONFIGURATON;
}

void system_controller_state_load_configuration(void)
{
	leds_set(2);
	printf("load configuration...\n");

	/* *** ENTRY *** */

	/* **** DO ***** */
	//if no valid configuration found, then run config terminal directly
	if(configuration_storage_init()) {
		next_state = STATE_WAITING_FOR_USER_INTERRUPT;
	} else {
		next_state = STATE_RUN_CONFIGURATION_TERMINAL;
	}

	/* *** EXIT **** */
}


void system_controller_state_waiting_for_user_interrupt(void)
{
	leds_set(3);
	printf("waiting for user interrupt...\n");

	/* *** ENTRY *** */
	bool user_irq_received = false;
	char user_input_char;

	uint8_t waiting_time = STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT;
	float delay = 1000.0/(STATE_WAITING_FOR_USER_INTERRUPT_PARTS + 1);

	uint8_t parts_of_seconds_counter = 0;

	printf("press space for entering configuration menu...\n");

	/* **** DO ***** */

	uart_flush();
	while(waiting_time > 0 && !user_irq_received) {

		//if user sends a space over usart then show configuration main menu
		while(uart_available()) {
			user_input_char = uart_getc();
			if(user_input_char == ' ') {
				user_irq_received = true;

				user_input_char = 0;
				uart_flush();
			}
		}

		//Display Counter and decreae timeout
		if(parts_of_seconds_counter == 0) {
			printf("%d", waiting_time);
			waiting_time--;
			parts_of_seconds_counter = STATE_WAITING_FOR_USER_INTERRUPT_PARTS;
		} else {
			printf(".");
			parts_of_seconds_counter--;
		}
		leds_toggle(0x80);
		_delay_ms(delay);
	}

	printf("\n");

	/* *** EXIT **** */
	if(user_irq_received) {
		user_irq_received = false;
		next_state = STATE_RUN_CONFIGURATION_TERMINAL;
	} else {
		next_state = STATE_INIT_CONTROLLER_ENVIRONMENT;
	}
	uart_flush();
}


void system_controller_state_run_configuration_terminal(void)
{
	/* *** ENTRY *** */

	printf("run configuration terminal...\n");
	leds_set(LED6 | LED7);
	/* **** DO ***** */

	//start sub state machine "configuration terminal"
	configuration_terminal_state_machine();

	/* *** EXIT **** */
	leds_set(0);
	next_state = STATE_WAITING_FOR_USER_INTERRUPT;
}


void system_controller_state_init_controller_environment(void)
{
	/* *** ENTRY *** */

	printf("init controller environment...\n");

	acceleration_vector_t acceleration_vector;
	angularvelocity_vector_t angularvelocity_vector;
	uint16_t angle_scalingfactor;
	float complementary_filter_ratio;
	float valid_accel_magnitude;

	/* **** DO ***** */

	//initialize PID Controller with saved values
	pid_robot_pos_config.p_factor = configuration_storage_get_pid_robot_pos_p_factor();
	pid_robot_pos_config.i_factor = configuration_storage_get_pid_robot_pos_i_factor();
	pid_robot_pos_config.d_factor = configuration_storage_get_pid_robot_pos_d_factor();
	pid_robot_pos_config.pid_scalingfactor = configuration_storage_get_pid_robot_pos_scalingfactor();


	pid_balance_config.p_factor = configuration_storage_get_pid_balance_p_factor();
	pid_balance_config.i_factor = configuration_storage_get_pid_balance_i_factor();
	pid_balance_config.d_factor = configuration_storage_get_pid_balance_d_factor();
	pid_balance_config.pid_scalingfactor = configuration_storage_get_pid_balance_scalingfactor();

//	pid_balance_edge_config.p_factor = configuration_storage_get_pid_edge_p_factor();
//	pid_balance_edge_config.i_factor = configuration_storage_get_pid_edge_i_factor();
//	pid_balance_edge_config.d_factor = configuration_storage_get_pid_edge_d_factor();
//	pid_balance_edge_config.pid_scalingfactor = configuration_storage_get_pid_edge_scalingfactor();
//
//	pid_balance_edge_angle = configuration_storage_get_pid_edge_angle();

	//TODO: HACK FIXME define variables in configuration storage
	pid_speed_motor_config.p_factor = configuration_storage_get_pid_speed_motor_p_factor();
	pid_speed_motor_config.i_factor = configuration_storage_get_pid_speed_motor_i_factor();
	pid_speed_motor_config.d_factor = configuration_storage_get_pid_speed_motor_d_factor();
	pid_speed_motor_config.pid_scalingfactor = configuration_storage_get_pid_speed_motor_scalingfactor();

	printf("robot pos: p:%6d i:%6d d:%6d s:%6d\n",
			pid_robot_pos_config.p_factor,
			pid_robot_pos_config.i_factor,
			pid_robot_pos_config.d_factor,
			pid_robot_pos_config.pid_scalingfactor);

	printf("balance: p:%6d i:%6d d:%6d s:%6d\n",
			pid_balance_config.p_factor,
			pid_balance_config.i_factor,
			pid_balance_config.d_factor,
			pid_balance_config.pid_scalingfactor);

	printf("motor speed: p:%6d i:%6d d:%6d s:%6d\n",
			pid_speed_motor_config.p_factor,
			pid_speed_motor_config.i_factor,
			pid_speed_motor_config.d_factor,
			pid_speed_motor_config.pid_scalingfactor);

	printf("angle stable: %u\n\n", angle_stable);


	//init pid
	pid_Init(5, 0, 3, 200,
			 &pid_robot_pos_data);


	pid_Init( pid_robot_pos_config.p_factor,
			  pid_robot_pos_config.i_factor,
			  pid_robot_pos_config.d_factor,
			  pid_robot_pos_config.pid_scalingfactor,
			 &pid_robot_speed_data);

	//init balance pid
	pid_Init( pid_balance_config.p_factor,
			  pid_balance_config.i_factor,
			  pid_balance_config.d_factor,
			  pid_balance_config.pid_scalingfactor,
			 &pid_balance_data);

	//init motor pids
	pid_Init( pid_speed_motor_config.p_factor,
			  pid_speed_motor_config.i_factor,
			  pid_speed_motor_config.d_factor,
			  pid_speed_motor_config.pid_scalingfactor,
			 &pid_speed_m1_data);

	pid_Init( pid_speed_motor_config.p_factor,
			  pid_speed_motor_config.i_factor,
			  pid_speed_motor_config.d_factor,
			  pid_speed_motor_config.pid_scalingfactor,
			 &pid_speed_m2_data);

	//init steering
	pid_Init(40, 0, 5, 10, &pid_steering_data);


	//set setpoint
	pid_robot_pos_setpoint	 	= 0;
	pid_robot_speed_setpoint	= 0;
	pid_balance_setpoint  	 	= 0;
	pid_speed_m1_setpoint    	= 0;
	pid_speed_m2_setpoint    	= 0;

	printf("pid_robot_speed setpoint: %d\n", pid_robot_pos_setpoint);
	printf("pid_balance     setpoint: %d\n", pid_balance_setpoint);
	printf("pid_speed_m1    setpoint: %d\n", pid_speed_m1_setpoint);
	printf("pid_speed_m2    setpoint: %d\n", pid_speed_m2_setpoint);

	//restore acceleration offset
	configuration_storage_get_acceleration_offset_vector(&acceleration_vector);
	motionsensor_set_acceleration_offset_vector(&acceleration_vector);
	printf("offset acceleration\n  x:%6d y:%6d z:%6d\n",
			acceleration_vector.x,
			acceleration_vector.y,
			acceleration_vector.z);

	//restore angularvelocity offset
	configuration_storage_get_angularvelocity_offset_vector(&angularvelocity_vector);
	motionsensor_set_angularvelocity_offset_vector(&angularvelocity_vector);
	printf("offset angularvelocity\n  x:%6d y:%6d z:%6d\n",
			angularvelocity_vector.x,
			angularvelocity_vector.y,
			angularvelocity_vector.z);

	//restore parameters for complementary filter
	complementary_filter_ratio = configuration_storage_get_complementary_filter_ratio();
	motionsensor_set_complementary_filter_ratio(complementary_filter_ratio);
	printf("complementary_filter_ratio\n  angularvelocity: %f, acceleration: %f\n",
			complementary_filter_ratio,
			MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE - complementary_filter_ratio);

	//restore scaling factor
	angle_scalingfactor = configuration_storage_get_angle_scalingfactor();
	motionsensor_set_angle_scalingfactor(angle_scalingfactor);
	printf("angle_scalingfactor: %d\n", angle_scalingfactor);

	//restore maximal acceleration magnitude
	valid_accel_magnitude = configuration_storage_get_valid_acceleration_magnitude();
	motionsensor_set_valid_acceleration_magnitude(valid_accel_magnitude);
	printf("valid_accel_magnitude: %f\n", valid_accel_magnitude);

	//init print_data_fptr
	print_mode = configuration_storage_get_print_data_mode();
	print_data_enabled = false;
	printf("print_data_mode: %d\n", print_mode);

	switch(print_mode) {

		case PRINT_NONE:
			print_data_fptr = NULL;
		break;

		case PRINT_DATA_ANGLEPID:
			print_data_fptr = &system_controller_print_data_anglepid;
		break;

		case PRINT_DATA_ALL_RAW:
			print_data_fptr = &system_controller_print_data_all_raw;
		break;

		case PRINT_DATA_ALL_FILTERED:
			print_data_fptr = &system_controller_print_data_all_filtered;
		break;

		case PRINT_DATA_REALLY_ALL_FILTERED:
			print_data_fptr = &system_controller_print_data_really_all_filtered;
		break;

		case PRINT_TICKER:
		default:
			print_data_fptr = &system_controller_print_ticker;
		break;
	}

	/* *** EXIT **** */

	next_state = STATE_INIT_REMAINING_HARDWARE;
}

void system_controller_state_init_remaining_hardware(void)
{
	uint8_t motor_acceleration;

	/* *** ENTRY *** */

	/* **** DO ***** */

	//init motor controller
	printf("init motor control...\n");
	motor_acceleration = configuration_storage_get_motor_acceleration();

	printf("motor acceleration: %d\n", motor_acceleration);
	motor_control_init(motor_acceleration);

	printf("init timers...\n");
	timer_init();							/* Init Timer */

	printf("will change uart speed to 250k...\n");
	while(uart_tx_buffer_size() > 0) {
		//wait, until tx buffer is empty
	}
	uart_init(UART_BAUDRATE_250k);

	/* *** EXIT **** */
	next_state = STATE_RUN_CONTROLLER;
}

static bool system_controller_exec_control;
//static motor_control_speed_t speed_remote_control;
static int16_t robot_speed;
static int32_t robot_pos;
static int32_t robot_real_pos;

static filter_moving_average_t robot_speed_avg;

static char command_buffer[COMMAND_BUFFER_MAX];
static int16_t command_speed;
static int16_t command_turn;
//static int16_t command_pid_balance_setpoint;

void system_controller_state_run_controller(void)
{
	/* *** ENTRY *** */
	printf("run controller...\n");
	leds_set(0);
	leds_color(LEDS_COLOR_BLACK);


	system_controller_exec_control = true;
	print_data_enabled = false;
	robot_pos = 0;
	robot_real_pos = 0;
	turning_offset = 0;
	filter_moving_average_init(&robot_speed_avg, 0);

	/* **** DO ***** */

	while(system_controller_exec_control) {

		/*
		 * - Read sensor values and calculate current angle
		 * - determine pid output
		 */
		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			//read angle
			current_angle = (motionsensor_angle_t)motionsensor_get_angle_y();

			//read speed and position
			robot_pos = motor_control_get_robot_position();
			robot_speed = motor_control_get_robot_speed();
			robot_real_pos = motor_control_get_real_robot_position();
			motor_control_get_current_speed(&current_speed);

			filter_moving_average_put_element(&robot_speed_avg, robot_speed);

#if 1
			pid_robot_speed_output = pid_Controller(pid_robot_speed_setpoint,
													robot_speed_avg.avg,
													&pid_robot_speed_data);

			if(pid_robot_speed_output > 24) {
				pid_robot_speed_output = 24;
			}

			if(pid_robot_speed_output < -24) {
				pid_robot_speed_output = -24;
			}

			pid_balance_setpoint = pid_robot_speed_output;



#else
			//calculate PID robot pos if not in halt zone
			if(robot_pos != 0) {
				leds_color(LEDS_COLOR_YELLOW);
				pid_robot_pos_output = pid_Controller(pid_robot_pos_setpoint,
													  robot_pos,
													  &pid_robot_pos_data);

				//limit pid output
				if(pid_robot_pos_output > PID_ROBOT_POS_OUTPUT_MAX) {
					pid_robot_pos_output = PID_ROBOT_POS_OUTPUT_MAX;
				}

				if(pid_robot_pos_output < PID_ROBOT_POS_OUTPUT_MIN) {
					pid_robot_pos_output = PID_ROBOT_POS_OUTPUT_MIN;
				}

				pid_balance_setpoint = pid_robot_pos_output;

			} else {
				//if in halt zone slow down robot
				pid_robot_speed_output = -(4 * robot_speed_avg.avg);

				leds_color(LEDS_COLOR_GREEN);

				//limit pid output
				if(pid_robot_speed_output > PID_ROBOT_SLOWING_OUTPUT_MAX) {
					pid_robot_speed_output = PID_ROBOT_SLOWING_OUTPUT_MAX;
				}

				if(pid_robot_speed_output < PID_ROBOT_SLOWING_OUTPUT_MIN) {
					pid_robot_speed_output = PID_ROBOT_SLOWING_OUTPUT_MIN;
				}

				pid_balance_setpoint = pid_robot_speed_output;
			}


			//test if robot is remote controlled
			if(timer_command_timeout != 0) {
				//remote controlled; use command_pid_balance_setpoint and reset robot_pos integral
				pid_balance_setpoint = command_pid_balance_setpoint;
				motor_control_reset_position();
				leds_color(LEDS_COLOR_BLUE);
			}

#endif

			if(timer_command_timeout > 0) {
				leds_color(LEDS_COLOR_BLUE);
				//remote controlled; use command_pid_balance_setpoint and reset robot_pos integral
				motor_control_reset_position();

			} else {
				leds_color(LEDS_COLOR_GREEN);

				command_speed = 0;
				command_turn = 0;
			}

			//calculate PID balance
			pid_balance_output = pid_Controller(pid_balance_setpoint,
												current_angle,
												&pid_balance_data);
			pid_balance_output = -pid_balance_output;

			pid_speed_m1_setpoint = pid_balance_output;
			pid_speed_m2_setpoint = pid_balance_output;


			//calculate PID motor speed
			pid_speed_m1_output = pid_Controller(pid_speed_m1_setpoint,
												 current_speed.motor_1,
												 &pid_speed_m1_data);

			pid_speed_m2_output = pid_Controller(pid_speed_m2_setpoint,
												 current_speed.motor_2,
												 &pid_speed_m2_data);

			//prepare new motor speed
			new_motor_speed.motor_1 = pid_speed_m1_output;
			new_motor_speed.motor_2 = pid_speed_m2_output;

			//remote control
			new_motor_speed.motor_1 += turning_offset;
			new_motor_speed.motor_2 -= turning_offset;


			motor_control_prepare_new_speed(&new_motor_speed);

		} //end TIMER_MAJORSLOT_0

		/*
		 * - set new motor speed
		 * - send robot data
		 * - parse command
		 */
		if(timer_current_majorslot == TIMER_MAJORSLOT_1) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			motor_control_set_new_speed();

			//print pid and angle
			if(print_data_enabled && print_data_fptr != NULL) {
				print_data_fptr();
			}

			system_controller_parse_command();

			//display speed on leds
			leds_set((uint8_t)abs(pid_balance_setpoint));
			//PORT_LEDS = (uint8_t)(abs(pid_balance_output));
			//PORT_LEDS = (uint8_t)(abs(pid_robot_pos_output));

			if(motor_control_motors_runaway()) {
				system_controller_exec_control = false;
			}

		} // end TIMER_MAJORSLOT_1

	} // end while(true)


	/* *** EXIT **** */

	//stop motors
	leds_color(LEDS_COLOR_RED);
	system_controller_halt_motors();
	leds_color(LEDS_COLOR_BLACK);

	next_state = STATE_LOAD_CONFIGURATON;
}

void system_controller_halt_motors(void)
{
	motor_control_speed_t speed;
	motor_control_get_new_speed(&speed);

	while(speed.motor_1 != 0 && speed.motor_2 != 0) {
		if(speed.motor_1 < 0) {
			speed.motor_1++;
		}

		if(speed.motor_1 > 0) {
			speed.motor_1--;
		}

		if(speed.motor_2 < 0) {
			speed.motor_2++;
		}

		if(speed.motor_2 > 0) {
			speed.motor_2--;
		}

		motor_control_prepare_new_speed(&speed);
		motor_control_set_new_speed();
		_delay_ms(2);
	}
}


void system_controller_parse_command(void)
{
	uint8_t cmd_len = uart_gets(command_buffer, COMMAND_BUFFER_MAX);
	int16_t speed_diff;

	if(cmd_len) {
		if(command_buffer[0] == 'S') {
			//reset timeout
			timer_command_timeout = COMMAND_TIMEOUT;

			switch (command_buffer[1]) {
				case 'F':

					if(command_speed < 16) {
						command_speed += 2;
					}
					break;

				case 'B':

					if(command_speed > -16) {
						command_speed -= 2;
					}
					break;

				case 'H':

					command_speed = 0;
					command_turn = 0;
					break;

				case 'R':

					if(command_turn < 21) {
						command_turn += 3;
					}
					break;

				case 'L':

					if(command_turn > -21) {
						command_turn -= 3;
					}
					break;

				default:
					break;
			}
		} else if((command_buffer[0] == 'K') && (command_buffer[1] == 'A')) {
			timer_command_timeout = COMMAND_TIMEOUT;
		} else if((command_buffer[0] == 'X') && (command_buffer[1] == '0')) {
			system_controller_exec_control = false;
		} else if((command_buffer[0] == 'D') && (command_buffer[1] == '1')) {
			print_data_enabled = true;
		} else if((command_buffer[0] == 'D') && (command_buffer[1] == '0')) {
			print_data_enabled = false;
		} else if((command_buffer[0] == 'R') && (command_buffer[1] == 'I')) {
			//TODO:
			pid_Reset_Integrator(&pid_robot_pos_data);
			pid_Reset_Integrator(&pid_balance_data);
			pid_Reset_Integrator(&pid_speed_m1_data);
			pid_Reset_Integrator(&pid_speed_m2_data);
		}

		//flush uart to prevent runover
		uart_flush();
	}

	//Update setpoints and speeds
	speed_diff = command_speed - pid_robot_speed_setpoint;
	if(speed_diff > 0) {
		pid_robot_speed_setpoint++;
	} else if(speed_diff < 0) {
		pid_robot_speed_setpoint--;
	}

	speed_diff = command_turn - turning_offset;
	if(speed_diff > 0) {
		turning_offset++;
	} else if(speed_diff < 0) {
		turning_offset--;
	}
}


void system_controller_print_ticker(void)
{
	if(print_ticker_cnt++ >= 250) {
		print_ticker_cnt = 0;
		printf(".\n");
	}
}

void system_controller_print_data_anglepid(void)
{
	printf("p:%d:%d\n",current_angle, pid_balance_output);
}


void system_controller_print_data_all_raw(void)
{
	uint8_t buf_idx = 0;

	motionsensor_motiondata_t motiondata;
	motionsensor_get_raw_motiondata(&motiondata);

	//header: type: r
	//header: size payload: 10
	//payload [accel.x H,accel.x L,accel.z H,accel.z L,angular.y H,angular.y L,angle H,angle L,pid H,pid L]
	print_data_buffer[buf_idx++] = 'r';
	print_data_buffer[buf_idx++] = 10;
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.x >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.x & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.z >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.z & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.angularvelocity.y >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.angularvelocity.y & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output & 0x00FF);

	//marshall packet with base64
	base64_encode(print_data_buffer, buf_idx);

	//send trough usart
	printf("%s\n",base64_encode_buffer);
}
static void system_controller_print_data_all_filtered(void)
{
	uint8_t buf_idx = 0;
	motionsensor_motiondata_t motiondata;
	motionsensor_get_filtered_motiondata(&motiondata);

	//header: type: f
	//header: size payload: 10
	//payload [accel.x H,accel.x L,accel.z H,accel.z L,angular.y H,angular.y L,angle H,angle L,pid H,pid L]
	print_data_buffer[buf_idx++] = 'f';
	print_data_buffer[buf_idx++] = 10;
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.x >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.x & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.z >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.z & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.angularvelocity.y >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.angularvelocity.y & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output & 0x00FF);

	//marshall packet with base64
	base64_encode(print_data_buffer, buf_idx);

	//send trough usart
	printf("%s\n",base64_encode_buffer);
}

static void system_controller_print_data_really_all_filtered(void)
{
	uint8_t buf_idx = 0;

	motionsensor_motiondata_t motiondata;
	motionsensor_get_filtered_motiondata(&motiondata);

	int16_t angle_accel = (int16_t)(motionsensor_get_angle_acceleration() * 1000.0);
	int8_t magnitude    = (int8_t)(motionsensor_get_angle_acceleration_magnitude() * 100.0);

	//header: type: A
	//header: size payload: 13
	//payload [accel.x H,accel.x L,accel.z H,accel.z L,angular.y H,angular.y L,magnitude,angle_accel H, angle_accel L, angle H,angle L,pid H,pid L]
	print_data_buffer[buf_idx++] = 'B';
	print_data_buffer[buf_idx++] = 13;
	print_data_buffer[buf_idx++] = (uint8_t)(current_speed.motor_1 >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(current_speed.motor_1 & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(current_speed.motor_2 >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(current_speed.motor_2 & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(robot_real_pos >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(robot_real_pos & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(turning_offset);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_robot_speed_setpoint >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_robot_speed_setpoint & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output & 0x00FF);

/*
 * 	print_data_buffer[buf_idx++] = 'A';
	print_data_buffer[buf_idx++] = 13;
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.x >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.x & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.z >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.acceleration.z & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.angularvelocity.y >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(motiondata.angularvelocity.y & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(magnitude);
	print_data_buffer[buf_idx++] = (uint8_t)(angle_accel >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(angle_accel & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(current_angle & 0x00FF);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output >> 8);
	print_data_buffer[buf_idx++] = (uint8_t)(pid_balance_output & 0x00FF);
 *
 */
	//marshall packet with base64
	base64_encode(print_data_buffer, buf_idx);

	//send trough usart
	printf("%s\n",base64_encode_buffer);
}
