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
volatile bool timer_slot_1;
volatile bool timer_slot_2;
volatile bool timer_slot_3;


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

static int16_t current_position;
static int16_t current_speed;
static acceleration_t current_acceleration;
static acceleration_t current_angularvelocity;

static pidData_t pid_data;

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

	UART_init(38400);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */
	timer_init();		/* Init Timer */

	DDR_LED = 0xFF;		/* Setze LED Port als Ausgang */
	DDR_SCOPE = 0xFF;

	sei();

	vt100_clear_all();

	printf("system_controller_state_init_hardware(void)\n");

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
	angularvelocity_t angularvelocity;
	uint16_t position_multiplier;

	double tmp_double;

	/* **** DO ***** */


	pid_Init(configuration_storage_get_p_factor(),
			 configuration_storage_get_i_factor(),
			 configuration_storage_get_d_factor(),
			 configuration_storage_get_scalingfactor(),
			 &pid_data);

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
	printf("system_controller_state_run_pid_controller(void)\n");

	//uint16_t speed = 0;
	motor_contol_speed_t new_speed;

	//double position = 1337;

	/* **** DO ***** */

	while(true) {

		if(timer_slot_1) {
			PORT_LED ^= _BV(7);

			timer_slot_1 = false;

			/*
			 * Sensorwerte lesen und in Position umrechnen
			 * als IST-Wert in den PID-Regler geben
			 * Stellgr��e an Motorsteuerung weitergeben, ABER noch nicht setzen
			 */
			current_position = motionsensor_get_position();
			current_speed = pid_Controller(0, current_position, &pid_data);

			if(current_speed > 125) {
				current_speed = 125;
			}

			if(current_speed < -125) {
				current_speed = -125;
			}

			new_speed.motor_1 = -current_speed;
			new_speed.motor_2 = -current_speed;

			motor_control_prepare_new_speed(&new_speed);
		}

		if(timer_slot_2) {

			timer_slot_2 = false;
			/*
			 * Neue Stellgröße des Motors setzen
			 */
			motor_control_set_new_speed();

			wireless_send_pid();
		}


		simplex_protocol_tick();

	} // end for(;;)


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
	wireless_send_buffer[1] = (uint8_t)(current_position >> 8);
	wireless_send_buffer[2] = (uint8_t)(current_position & 0x00FF);

	//speed
	wireless_send_buffer[3] = (uint8_t)((current_speed) >> 8);
	wireless_send_buffer[4] = (uint8_t)((current_speed) & 0x00FF);

	wireless_send_buffer_len = 5;

	simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
			wireless_send_buffer_len,
			wireless_send_buffer);
}
