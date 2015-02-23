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
#include "lib/rfm12.h"

#include "configuration_terminal.h"
#include "configuration_storage.h"
#include "vt100.h"

#include "motionsensor.h"
#include "simplex-protocol.h"
#include "motor_control.h"
#include "pid.h"
#include "timer.h"
#include "leds.h"


/* *** DECLARATIONS ********************************************************** */


/* global types and constants */
// Timer
volatile timer_slot_t timer_current_majorslot;
volatile timer_slot_t timer_current_minorslot;

/* local type and constants     */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4


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

static int16_t current_angle;
static int16_t pid_output;
static motor_contol_speed_t new_motor_speed;

static pid_config_t pid_center;
static pid_config_t pid_edge;
static uint16_t		pid_edge_angle;

static pidData_t pid_controller_data;
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
//static inline void wireless_send_acceleration(void);
//static inline void wireless_send_angularvelocity(void);
//static inline void wireless_send_pid(void);

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

	//UART_init(112500);						/* Init UART mit 112500 baud */
	UART_init();
	twi_master_init(TWI_TWBR_VALUE_400);	/* Init TWI/I2C Schnittstelle */
	timer_init();							/* Init Timer */
	leds_init();

	sei();

	vt100_clear_all();
	printf("basic hardware inited...\n");

	//init rfm12 interface
	//rfm12_init();

	//init simplex_protocol
	//simplex_protocol_init();


	//init motionsensor
	printf("init motionsensor...\n");
	motionsensor_init();

	//init motor controller
	printf("init motor control...\n");
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

	printf("press any key for entering configuration menu...\n");

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
		PORT_LEDS ^= _BV(LED0);
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

	//set setpoint
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


	uint8_t led = 0;

	uint8_t sensordata[5];

	int i;


	while(true) {

		/*
		 * - Read sensor values and calculate current angle
		 * - determine pid output
		 * - limit pid output and prepare as new motor speed
		 */
		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			leds_inc();

			//read angle

			PORT_LEDS |= _BV(LED6);
			current_angle = motionsensor_get_angle_y();
			PORT_LEDS&= ~_BV(LED6);

			//calculate pid

			PORT_LEDS |= _BV(LED5);
			pid_output = pid_Controller(pid_setpoint, current_angle, &pid_controller_data);
			PORT_LEDS&= ~_BV(LED5);

//			PORT_LEDS &= ~_BV(LED6);
//			printf("#%d:%d\n",current_angle, pid_output);
//			PORT_LEDS |= _BV(LED6);



			//limit pid output for motor control
			/*
			if(pid_output > 255) {
				PORT_LED |= _BV(7);
				pid_output = 255;
			} else if(pid_output < -255) {
				pid_output = -255;
				PORT_LED |= _BV(6);
			}
			*/

			//display on LEDs
//			if(pid_output >= 0) {
//				PORT_LED = pid_output;
//			} else {
//				PORT_LED = -pid_output;
//			}

			//prepare new motor speed
			PORT_LEDS |= _BV(LED4);
			new_motor_speed.motor_1 = pid_output;
			new_motor_speed.motor_2 = pid_output;
			motor_control_prepare_new_speed(&new_motor_speed);
			PORT_LEDS&= ~_BV(LED4);


		} //end TIMER_MAJORSLOT_0

		/*
		 * - set new motor speed
		 * - prepare send pid data
		 */
		if(timer_current_majorslot == TIMER_MAJORSLOT_1) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;
			PORT_LEDS |= _BV(LED3);
			motor_control_set_new_speed();
			PORT_LEDS&= ~_BV(LED3);
		} // end TIMER_MAJORSLOT_1

	} // end while(true)


	/* *** EXIT **** */

	next_state = STATE_NULL;
}
