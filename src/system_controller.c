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
	STATE_INIT_BASIC_HARDWARE,
	STATE_LOAD_CONFIGURATON,
	STATE_WAITING_FOR_USER_INTERRUPT,
	STATE_RUN_CONFIGURATION_TERMINAL,
	STATE_INIT_CONTROLLER_ENVIRONMENT,
	STATE_INIT_REMAINING_HARDWARE,
	STATE_RUN_CONTROLLER,
	STATE_RUN_DEBUG,
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
static void system_controller_state_init_basic_hardware(void);
static void system_controller_state_load_configuration(void);
static void system_controller_state_waiting_for_user_interrupt(void);
static void system_controller_state_run_configuration_terminal(void);
static void system_controller_state_init_controller_environment(void);
static void system_controller_state_init_remaining_hardware(void);
static void system_controller_state_run_controller(void);
static void system_controller_state_run_debug_mode(void);

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

			case STATE_RUN_DEBUG:
				system_controller_state_run_debug_mode();
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
	UART_init();							/* Init USART */
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
		next_state = STATE_INIT_CONTROLLER_ENVIRONMENT;
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


void system_controller_state_init_controller_environment(void)
{
	/* *** ENTRY *** */

	printf("init controller enviroment...\n");

	acceleration_vector_t acceleration_vector;
	angularvelocity_vector_t angularvelocity_vector;
	uint16_t angle_scalingfactor;
	uint8_t complementary_filter_ratio;

	/* **** DO ***** */

	//initialize PID Controller with saved values
	pid_center.p_factor = configuration_storage_get_pid_center_p_factor();
	pid_center.i_factor = configuration_storage_get_pid_center_i_factor();
	pid_center.d_factor = configuration_storage_get_pid_center_d_factor();
	pid_center.pid_scalingfactor = configuration_storage_get_pid_center_scalingfactor();

	pid_edge.p_factor = configuration_storage_get_pid_edge_p_factor();
	pid_edge.i_factor = configuration_storage_get_pid_edge_i_factor();
	pid_edge.d_factor = configuration_storage_get_pid_edge_d_factor();
	pid_edge.pid_scalingfactor = configuration_storage_get_pid_edge_scalingfactor();

	pid_edge_angle = configuration_storage_get_pid_edge_angle();

	printf("center pid: p:%6d i:%6d d:%6d s:%6d\n",
			pid_center.p_factor,
			pid_center.i_factor,
			pid_center.d_factor,
			pid_center.pid_scalingfactor);

	printf("edge pid  : p:%6d i:%6d d:%6d s:%6d\n",
			pid_center.p_factor,
			pid_center.i_factor,
			pid_center.d_factor,
			pid_center.pid_scalingfactor);

	printf("edge angle: %u\n", pid_edge_angle);


	//init pid with parameters for center pid
	pid_Init(pid_center.p_factor,
			 pid_center.i_factor,
			 pid_center.d_factor,
			 pid_center.pid_scalingfactor,
			 &pid_controller_data);

	//set setpoint
	pid_setpoint = 0;

	printf("pid setpoint: %d\n", pid_setpoint);

	//restore scaling factor
	angle_scalingfactor = configuration_storage_get_angle_scalingfactor();
	motionsensor_set_angle_scalingfactor(angle_scalingfactor);
	printf("angle_scalingfactor: %d\n", angle_scalingfactor);

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
	printf("complementary_filter_ratio\n  angularvelocity: %d, acceleration: %d\n",
			complementary_filter_ratio,
			100 - complementary_filter_ratio);


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


	/* *** EXIT **** */
	if(configuration_storage_get_run_mode() == CONFIGURATION_STORAGE_RUN_MODE_DEBUG) {
		next_state = STATE_RUN_DEBUG;
	} else {
		next_state = STATE_RUN_CONTROLLER;
	}

}

void system_controller_state_run_controller(void)
{
	/* *** ENTRY *** */
	printf("run controller...\n");
	PORT_LEDS = 0x00;

	/* **** DO ***** */

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

void system_controller_state_run_debug_mode(void)
{
	uint8_t led_value = 0;

	/* *** ENTRY *** */
	printf("run debug mode...\n");

	/* **** DO ***** */
/*	while(true) {
		motionsensor_printdata();
		//PORT_LEDS = led_value++;
		_delay_ms(40);
	}
*/
	while(true) {
		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			//PORT_LEDS = 0xFF;
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			motionsensor_printdata();
			//PORT_LEDS = 0;
		}
	}


	/* *** EXIT **** */

	next_state = STATE_NULL;
}
