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

#include <util/delay.h>

#include "lib/uart.h"
#include "lib/twi_master.h"
#include "lib/rfm12.h"

#include "configuration_terminal.h"
#include "configuration_storage.h"
#include "vt100.h"

#include "motionsensor.h"
#include "simplex-protocol.h"
#include "motor_control.h"
#include "pid.h"
#include "timer.h"
#include "common.h"


/* *** DECLARATIONS ********************************************************** */




/* global types and constants */
// Timer
volatile bool timer_slot_0;
volatile bool timer_slot_1;



/* local type and constants     */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4

//Datatypes iterate higher nibble - lower nibble for sec-number
#define WIRELESS_TYPE_DATA_PID				0x80
#define WIRELESS_TYPE_DATA_ACCELERATION		0x90
#define WIRELESS_TYPE_DATA_ANGULARVELOCITY	0xA0

#define WIRELESS_SEND_BUFFER_MAX_LEN		10

typedef enum {
	STATE_INIT_HARDWARE,
	STATE_LOAD_CONFIGURATON,
	STATE_WAITING_FOR_USER_INTERRUPT,
	STATE_RUN_CONFIGURATION_TERMINAL,
	STATE_INIT_PID_CONTROLLER,
	STATE_RUN_PID_CONTROLLER,
	STATE_FINAL,
	STATE_NULL
} system_controller_state_t;

/* * local objects               * */
static system_controller_state_t current_state;
static system_controller_state_t next_state;

static uint8_t wireless_send_buffer[WIRELESS_SEND_BUFFER_MAX_LEN];
static uint8_t wireless_send_buffer_len = 0;

static int16_t current_angle;
static int16_t pid_output;
static motor_contol_speed_t new_motor_speed;
static acceleration_t current_acceleration;
static acceleration_t current_angularvelocity;

static pidData_t pid_data;
static int8_t    pid_setpoint;

/* local function declarations  */


// states
static void system_controller_state_init_hardware(void);
static void system_controller_state_load_configuration(void);
static void system_controller_state_waiting_for_user_interrupt(void);
static void system_controller_state_run_configuration_terminal(void);
static void system_controller_state_init_pid_controller(void);
static void system_controller_state_run_pid_controller(void);


//inline helper functions for sending data
static inline void wireless_send_acceleration(void);
static inline void wireless_send_angularvelocity(void);
static inline void wireless_send_pid(void);

/* *** FUNCTION DEFINITIONS ************************************************** */
void system_controller_state_machine(void)
{
	current_state = STATE_INIT_HARDWARE;
	next_state = STATE_NULL;

	while(current_state != STATE_FINAL) {
		switch(current_state) {

			case STATE_INIT_HARDWARE:
				system_controller_state_init_hardware();
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

			case STATE_INIT_PID_CONTROLLER:
				system_controller_state_init_pid_controller();
			break;

			case STATE_RUN_PID_CONTROLLER:
				system_controller_state_run_pid_controller();
			break;

			default:
				current_state = STATE_FINAL;
			break;

		}

		//change state
		current_state = next_state;
	}
}

void system_controller_state_init_hardware(void)
{
	/* *** ENTRY *** */



	/* **** DO ***** */

	UART_init(56700);						/* Init UART mit 38400 baud */
	twi_master_init(TWI_TWBR_VALUE_100);	/* Init TWI/I2C Schnittstelle */
	timer_init();							/* Init Timer */

	DDR_LED = 0xFF;							/* Set LED port as output */
	DDR_SCOPE = 0xFF;						/* set scope port as output */

	sei();

	vt100_clear_all();

	printf("init hardware...\n");

	//init rfm12 interface
	rfm12_init();

	//init simplex_protocol
	simplex_protocol_init();

	//init motionsensor
	motionsensor_init();

	//init motor controller
	motor_control_init();


	/* *** EXIT **** */

	next_state = STATE_LOAD_CONFIGURATON;
	//next_state = STATE_RUN_PID_CONTROLLER;
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


	//configuration_terminal_clear_all();

	printf("Press any key for entering configuration menu...\n");

	/* **** DO ***** */

	UART_clr_rx_buf();
	while(waiting_time > 0 && !user_irq_received) {


		//if user send any byte over usart then show configuration main menu
		if(UART_char_received()) {
			user_irq_received = true;
			UART_clr_rx_buf();
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
		PORT_LED ^= _BV(0);
		_delay_ms(delay);
	}

	printf("\n");

	/* *** EXIT **** */
	if(user_irq_received) {
		user_irq_received = false;
		next_state = STATE_RUN_CONFIGURATION_TERMINAL;
	} else {
		next_state = STATE_INIT_PID_CONTROLLER;
	}
	vt100_clear_input_buffer();
}


void system_controller_state_run_configuration_terminal(void)
{
	/* *** ENTRY *** */

	printf("run configuration terminal...\n");

	/* **** DO ***** */
	//start sub state machine "configuration terminal"
	configuration_terminal_state_machine();

	/* *** EXIT **** */

	next_state = STATE_WAITING_FOR_USER_INTERRUPT;
	vt100_clear_all();
}


void system_controller_state_init_pid_controller(void)
{
	/* *** ENTRY *** */

	printf("init pid controller...\n");

	acceleration_t acceleration;
	angularvelocity_t angularvelocity;
	uint16_t position_multiplier;

	double tmp_double;

	/* **** DO ***** */

	//initialize PID Controller with saved values
	pid_Init(configuration_storage_get_p_factor(),
			 configuration_storage_get_i_factor(),
			 configuration_storage_get_d_factor(),
			 configuration_storage_get_scalingfactor(),
			 &pid_data);

	//set set_point
	pid_setpoint = 0;

	//restore position multiplier
	position_multiplier = configuration_storage_get_position_multiplier();
	motionsensor_set_position_multiplier(position_multiplier);

	//restore acceleration offset
	configuration_storage_get_acceleration_offset(&acceleration);
	motionsensor_set_acceleration_offset(&acceleration);

	//restore angularvelocity offset
	configuration_storage_get_angularvelocity_offset(&angularvelocity);
	motionsensor_set_angularvelocity_offset(&angularvelocity);

	//restore parameters for complementary filter
	tmp_double = configuration_storage_get_complementary_filter_acceleraton_factor();
	motionsensor_set_complementary_filter_acceleraton_factor(tmp_double);
	tmp_double = configuration_storage_get_complementary_filter_angularvelocity_factor();
	motionsensor_set_complementary_filter_angularvelocity_factor(tmp_double);

	/* *** EXIT **** */

	next_state = STATE_RUN_PID_CONTROLLER;
}


void system_controller_state_run_pid_controller(void)
{
	/* *** ENTRY *** */
	printf("run controller loop...\n");

	/* **** DO ***** */

	twi_master_set_speed(TWI_TWBR_VALUE_400);		//Set TWI speed to 400 khz for reading sensor

	PORT_LED = 0x00;
	PORT_SCOPE = 0x00;

	while(true) {

		/*
		 * - Read sensor values and calculate current angle
		 * - determine pid output
		 * - limit pid output and prepare as new motor speed
		 */
		if(timer_slot_0) {
			PORT_SCOPE = 0x01;
			timer_slot_0 = false;

			//read angle
			current_angle = motionsensor_get_angle();

			//calculate pid
			pid_output = pid_Controller(pid_setpoint, current_angle, &pid_data);



			//limit pid output for motor control
			if(pid_output > INT8_MAX) {
				pid_output = INT8_MAX;
			} else if(pid_output < INT8_MIN) {
				pid_output = INT8_MIN;
			}

			if(pid_output >= 0) {
				PORT_LED = pid_output;
			} else {
				PORT_LED = -pid_output;
			}

			//prepare new motor speed
			//TODO: PID Outpxut shouldn't inverted. something wrong named in mpu9150 module?
			new_motor_speed.motor_1 = -pid_output;
			new_motor_speed.motor_2 = -pid_output;
			motor_control_prepare_new_speed(&new_motor_speed);

			twi_master_set_speed(TWI_TWBR_VALUE_100);	//Set TWI speed to 100 khz for setting motor speed in next time slot

			PORT_SCOPE = 0x00;
		} //end time_slot_0

		/*
		 * - set new motor speed
		 * - prepare send pid data
		 */
		if(timer_slot_1) {
			PORT_SCOPE = 0x02;
			timer_slot_1 = false;

			motor_control_set_new_speed();

			twi_master_set_speed(TWI_TWBR_VALUE_400);	//Set TWI speed to 400 khz for reading sensor in next time slot

			//wireless_send_pid();
			//simplex_protocol_tick();
			PORT_SCOPE = 0x00;
		} // end time_slot_1
	} // end while(true)


	/* *** EXIT **** */

	next_state = STATE_NULL;
}

static inline void wireless_send_acceleration(void)
{
	wireless_send_buffer[0] = WIRELESS_TYPE_DATA_ACCELERATION;

	wireless_send_buffer[1] = (uint8_t)(current_acceleration.x >> 8);
	wireless_send_buffer[2] = (uint8_t)(current_acceleration.x & 0x00FF);

	//acceleration_y
	wireless_send_buffer[3] = (uint8_t)((current_acceleration.y) >> 8);
	wireless_send_buffer[4] = (uint8_t)((current_acceleration.y) & 0x00FF);

	//acceleration_z
	wireless_send_buffer[5] = (uint8_t)((current_acceleration.z) >> 8);
	wireless_send_buffer[6] = (uint8_t)((current_acceleration.z) & 0x00FF);

	wireless_send_buffer_len = 7;


	simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
			wireless_send_buffer_len,
			wireless_send_buffer);
}

static inline void wireless_send_angularvelocity(void)
{
	wireless_send_buffer[0] = WIRELESS_TYPE_DATA_ANGULARVELOCITY;

	wireless_send_buffer[1] = (uint8_t)(current_angularvelocity.x >> 8);
	wireless_send_buffer[2] = (uint8_t)(current_angularvelocity.x & 0x00FF);

	//acceleration_y
	wireless_send_buffer[3] = (uint8_t)((current_angularvelocity.y) >> 8);
	wireless_send_buffer[4] = (uint8_t)((current_angularvelocity.y) & 0x00FF);

	//acceleration_z
	wireless_send_buffer[5] = (uint8_t)((current_angularvelocity.z) >> 8);
	wireless_send_buffer[6] = (uint8_t)((current_angularvelocity.z) & 0x00FF);


	wireless_send_buffer_len = 7;


	simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
			wireless_send_buffer_len,
			wireless_send_buffer);
}

static inline void wireless_send_pid(void)
{


	//header
	wireless_send_buffer[0] = WIRELESS_TYPE_DATA_PID;
/*	uint32_t temp;


	//position
	temp = (uint32_t)(current_position);
	wireless_send_buffer[1] = (uint8_t)(temp >> 24);

	temp = (uint32_t)(current_position);
	temp = temp & 0x00FF0000;
	wireless_send_buffer[2] = (uint8_t)(temp >> 16);

	temp = (uint32_t)(current_position);
	temp = temp & 0x0000FF00;
	wireless_send_buffer[3] = (uint8_t)(temp >> 8);

	temp = (uint32_t)(current_position);
	wireless_send_buffer[4] = (uint8_t)((temp) & 0x000000FF);
*/
	//position
	wireless_send_buffer[1] = (uint8_t)(current_angle >> 8);
	wireless_send_buffer[2] = (uint8_t)(current_angle & 0x00FF);

	//speed
	wireless_send_buffer[3] = (uint8_t)((pid_output) >> 8);
	wireless_send_buffer[4] = (uint8_t)((pid_output) & 0x00FF);

	wireless_send_buffer_len = 5;

	simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
			wireless_send_buffer_len,
			wireless_send_buffer);
}
