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

#include "configuration_terminal.h"
#include "configuration_storage.h"
#include "vt100.h"


#include "motionsensor.h"
#include "simplex-protocol.h"
#include "motor_control.h"
#include "pid.h"
#include "timer.h"


/* *** DECLARATIONS ********************************************************** */

// Timer
volatile bool timer_compare_reached;
volatile uint8_t timer_slot_counter;


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


static system_controller_state_t current_state;
static system_controller_state_t next_state;

static pidData_t pid_data;

/* local function declarations  */


// states
static void system_controller_state_init_hardware(void);
static void system_controller_state_load_configuration(void);
static void system_controller_state_waiting_for_user_interrupt(void);
static void system_controller_state_run_configuration_terminal(void);
static void system_controller_state_init_pid_controller(void);
static void system_controller_state_run_pid_controller(void);

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

	UART_init(38400);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */
	timer_init();		/* Init Timer */

	sei();

	vt100_clear_all();

	printf("system_controller_state_init_hardware(void)\n");

	//init rfm12 interface
	simplex_protocol_init();

	//init motionsensor
	motionsensor_init();

	//init motor controller
//	motor_control_init();


	/* *** EXIT **** */

	next_state = STATE_LOAD_CONFIGURATON;
}

void system_controller_state_load_configuration(void)
{
	printf("system_controller_state_load_configuration(void)\n");

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
	printf("system_controller_state_waiting_for_user_interrupt(void)\n");

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

		_delay_ms(delay);
	}

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

	printf("system_controller_state_run_configuration_terminal(void)\n");

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

	printf("system_controller_state_init_pid_controller(void)\n");

	acceleration_t acceleration;
	uint16_t position_multiplier;

	/* **** DO ***** */


	pid_Init(configuration_storage_get_p_factor(),
			 configuration_storage_get_i_factor(),
			 configuration_storage_get_d_factor(),
			 configuration_storage_get_scalingfactor(),
			 &pid_data);

	configuration_storage_get_acceleration_offset(&acceleration);
	motionsensor_set_acceleration_offset(&acceleration);

	position_multiplier = configuration_storage_get_position_multiplier();
	motionsensor_set_position_multiplier(position_multiplier);

	/* *** EXIT **** */

	next_state = STATE_RUN_PID_CONTROLLER;
}


void system_controller_state_run_pid_controller(void)
{
	/* *** ENTRY *** */
	printf("system_controller_state_run_pid_controller(void)\n");

	uint16_t speed = 0;
	motor_contol_speed_t new_speed;

	double position;

	acceleration_t accel_tmp;

	/* **** DO ***** */

	for(;;) {
		if(timer_compare_reached) {

			timer_compare_reached = false;
			//led_value ^= LED3;

			if(timer_slot_counter == 0) {

				motionsensor_get_current_acceleration(&accel_tmp);

				printf("%d,%d;%d\n", accel_tmp.x, accel_tmp.y, accel_tmp.z);

			}

			/* Erster Zeitslot am Anfang des Intervalls; t = 0 ms */
			if(timer_slot_counter == 0) {
				/*
				 * Beschleunigungswerte lesen
				 * Beschleunigungswerte in Position umrechnen
				 * als IST-Wert in den PID-Regler geben
				 * Stellgr��e an Motorsteuerung weitergeben, ABER noch nicht setzen
				 */
//				PORTC ^= (LED1 | LED2);				//LED1 an.

				//Beschleunigungswerte lesen und in Position umrechnen
//				position = motionsensor_get_position();

				//Aktuelle Position an den PID Regler geben und neue Stellgr��e berechnen
//				speed = pid_Controller(0, (int16_t)(position), &pid_data);

//				new_speed.motor_1 = speed >> 8;
//				new_speed.motor_2 = speed >> 8;

				// Neue Stellgr��e an Motorsteuerung weitergeben, ABER noch nicht setzen
//				motor_control_prepare_new_speed(&new_speed);
//				PORTC ^= LED2;
			}

			/* Zweiter Zeitslot des Intervalls; t = 12 ms */
			if(timer_slot_counter == 3) {
//				PORTC ^= LED2;
				/*
				 * Neue Stellgr��e des Motors setzen
				 */
//				motor_control_set_new_speed();
//				PORTC ^= LED2;
			}

//			if(timer_slot_counter >= TIMER_SLOT_COUNTER_MAX) {
//				led_value &= ~(LED1 | LED2);	//Wieder beide LEDs l�schen
//			}
		}
	}

	/* *** EXIT **** */

	next_state = STATE_NULL;
}
