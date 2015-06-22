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

#include "lib/uart.h"
#include "lib/twi_master.h"

#include "configuration_terminal.h"
#include "configuration_storage.h"
#include "vt100.h"

#include "motionsensor.h"
#include "motor_control.h"
#include "pid.h"
#include "timer.h"
#include "base64.h"
#include "leds.h"
#include "encoder.h"


/* *** DECLARATIONS ********************************************************** */


/* global types and constants */
// Timer
volatile timer_slot_t timer_current_majorslot;
volatile timer_slot_t timer_current_minorslot;

/* local type and constants     */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4

#define PRINT_DATA_BUFFER_SIZE 	20

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

static int16_t pid_robot_pos_output;	//angle setpoint
static int16_t pid_balance_output;		//speed for motors
static int16_t pid_speed_m1_output;		//pwm for drivers
static int16_t pid_speed_m2_output;		//pwm for drivers

static motor_contol_speed_t current_speed;
static motor_contol_speed_t new_motor_speed;

static pid_config_t				pid_robot_pos_config;
static pid_config_t				pid_balance_config;
static pid_config_t				pid_speed_motor_config;

static motionsensor_angle_t		angle_stable;

static pidData_t pid_robot_pos_data;
static pidData_t pid_balance_data;
static pidData_t pid_speed_m1_data;
static pidData_t pid_speed_m2_data;

static int16_t	 pid_robot_pos_setpoint;
static int16_t   pid_balance_setpoint;
static int16_t	 pid_speed_m1_setpoint;
static int16_t	 pid_speed_m2_setpoint;

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
	uart_init();							/* Init USART */
	printf("usart inited\n");

	printf("init LED Port...\n");
	leds_init();


	printf("init TWI interface...\n");
	twi_master_init(TWI_TWBR_VALUE_400);	/* Init TWI/I2C Schnittstelle */

	printf("enable interrupts...\n");
	sei();

	//init motionsensor
	printf("init motionsensor...\n");
	motionsensor_init();

	/* *** EXIT **** */

	next_state = STATE_LOAD_CONFIGURATON;
}

void system_controller_state_load_configuration(void)
{
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
	printf("waiting for user interrupt...\n");

	/* *** ENTRY *** */
	bool user_irq_received = false;

	uint8_t waiting_time = STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT;
	double delay = 1000.0/(STATE_WAITING_FOR_USER_INTERRUPT_PARTS + 1);

	uint8_t parts_of_seconds_counter = 0;

	printf("press any key for entering configuration menu...\n");

	/* **** DO ***** */

	uart_clr_rx_buf();
	while(waiting_time > 0 && !user_irq_received) {

		//if user send any byte over usart then show configuration main menu
		if(uart_char_received()) {
			user_irq_received = true;
			uart_clr_rx_buf();
			break;
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
		PORT_LEDS ^= _BV(LED0);
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
	vt100_clear_input_buffer();
}


void system_controller_state_run_configuration_terminal(void)
{
	/* *** ENTRY *** */

	printf("run configuration terminal...\n");
	PORT_LEDS =    _BV(LED6) | _BV(LED7);
	/* **** DO ***** */

	//start sub state machine "configuration terminal"
	configuration_terminal_state_machine();

	/* *** EXIT **** */
	PORT_LEDS &= ~(_BV(LED6) | _BV(LED7));
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



	//init robot_speed pid
	pid_Init( pid_robot_pos_config.p_factor,
			  pid_robot_pos_config.i_factor,
			  pid_robot_pos_config.d_factor,
			  pid_robot_pos_config.pid_scalingfactor,
			 &pid_robot_pos_data);

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


	//set setpoint
	pid_robot_pos_setpoint	 = 0;
	pid_balance_setpoint  	 = 0;
	pid_speed_m1_setpoint    = 0;
	pid_speed_m2_setpoint    = 0;

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

	print_data_enum_t print_mode = configuration_storage_get_print_data_mode();
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

	printf("init motor encoders...\n");
	encoder_init();

	printf("init timers...\n");
	timer_init();							/* Init Timer */

	/* *** EXIT **** */
	next_state = STATE_RUN_CONTROLLER;
}

void system_controller_state_run_controller(void)
{
	/* *** ENTRY *** */
	printf("run controller...\n");
	PORT_LEDS = 0x00;

	int64_t robot_pos = 0; //temporary integrator

	/* **** DO ***** */

	while(true) {

		/*
		 * - Read sensor values and calculate current angle
		 * - determine pid output
		 */
		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			//read angle
			current_angle = (motionsensor_angle_t)motionsensor_get_angle_y();

			//read encoder speed
			current_speed.motor_1 = -encoder_read_delta(ENCODER_M1); //invert encoder signal, because m2 turns invers to m1
			current_speed.motor_2 =  encoder_read_delta(ENCODER_M2);

			//integrate to position
			//TODO: implement overflow protection!!!
			//robot_pos += (current_speed.motor_2 - current_speed.motor_1) / 2;
			//robot_pos = -robot_pos;

			//switch pid set
//			if(abs(current_angle) > pid_edge_angle) {
//				pid_controller_data.P_Factor = pid_edge.p_factor;
//				pid_controller_data.I_Factor = pid_edge.i_factor;
//				pid_controller_data.D_Factor = pid_edge.d_factor;
//			} else {
//				pid_controller_data.P_Factor = pid_center.p_factor;
//				pid_controller_data.I_Factor = pid_center.i_factor;
//				pid_controller_data.D_Factor = pid_center.d_factor;
//			}

			//calculate PID robot pos
			pid_robot_pos_output = pid_Controller(pid_robot_pos_setpoint,
													robot_pos,
													&pid_robot_pos_data);

			//TODO: Implement heartbeat and switch mode
			if(abs(current_angle) > angle_stable) {
			//if(true) {
				pid_balance_setpoint = pid_robot_pos_output;
			} else {
				robot_pos = 0;			//reset robot_pos integral
				pid_balance_setpoint = 0;
			}

			//calculate PID balance
			pid_balance_output =  pid_Controller(pid_balance_setpoint,
												current_angle,
												&pid_balance_data);
			pid_speed_m1_setpoint = -pid_balance_output;
			pid_speed_m2_setpoint = -pid_balance_output;

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

			motor_control_prepare_new_speed(&new_motor_speed);

		} //end TIMER_MAJORSLOT_0

		/*
		 * - set new motor speed
		 * - prepare send pid data
		 */
		if(timer_current_majorslot == TIMER_MAJORSLOT_1) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			//motor_control_set_new_speed();

			//print pid and angle
			if(print_data_fptr != NULL) {
				print_data_fptr();
			}

			//system_controller_parse_command();

			//display speed on leds
			//PORT_LEDS = (uint8_t)(abs(pid_balance_output));
			//PORT_LEDS = (uint8_t)(abs(pid_robot_pos_output));

		} // end TIMER_MAJORSLOT_1

	} // end while(true)


	/* *** EXIT **** */

	next_state = STATE_NULL;
}

void system_controller_print_ticker(void)
{
	if(print_ticker_cnt++ >= 250) {
		print_ticker_cnt = 0;
		printf(".\n");
	}
}

static int8_t m1 = 0;
static int8_t m2 = 0;
static int8_t leds = 0;

#define CMD_BUFFER_MAX 20
static char command_buffer[CMD_BUFFER_MAX];


void system_controller_parse_command(void)
{

	//UART_char_received();
	//fgets(command_buffer, CMD_BUFFER_MAX, stdin);

	//PORT_LEDS = leds++;

	//printf("hello world\n");

	/*

	if(UART_char_received()) {
		fgets(command_buffer, CMD_BUFFER_MAX, stdin);

		PORT_LEDS = strlen(command_buffer);

		fflush(stdin);
		UART_clr_rx_buf();
		//vt100_clear_input_buffer();
		command_buffer[0] = '\0';
	}
	*/
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
	print_data_buffer[buf_idx++] = 'A';
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

	//marshall packet with base64
	base64_encode(print_data_buffer, buf_idx);

	//send trough usart
	printf("%s\n",base64_encode_buffer);
}
