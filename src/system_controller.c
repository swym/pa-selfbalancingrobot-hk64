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

#include <avr/eeprom.h>
#include <util/delay.h>

#include "lib/uart.h"
#include "lib/twi_master.h"

#include "configuration_setting.h"
#include "vt100.h"


/* *** DECLARATIONS ********************************************************** */

/* import global variables */
configuration_setting_t configuration_setting_data_eeprom[CONFIGURATION_SETTING_COUNT]	__attribute__ ((section (".eeprom")));
uint8_t configuration_setting_current_index_eeprom										__attribute__ ((section (".eeprom")));

configuration_setting_t configuration_setting_default;
configuration_setting_t configuration_setting_data[CONFIGURATION_SETTING_COUNT];
uint8_t configuration_setting_current_index;



/* local type and constants     */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4


typedef enum {
	STATE_INIT_COMMUNICATION_INTERFACES,
	STATE_LOAD_SETTINGS,
	STATE_WAITING_FOR_USER_INTERRUPT,
	STATE_RUN_CONFIGURATION_TERMINAL,
	STATE_INIT_PID_CONTROLLER,
	STATE_INIT_TIMER,
	STATE_RUN_PID_CONTROLLER,
	STATE_INIT,
	STATE_FINAL,
	STATE_NULL
} system_controller_state_t;


static bool state_machine_running = true;
static system_controller_state_t current_state = STATE_INIT_COMMUNICATION_INTERFACES;
static system_controller_state_t next_state = STATE_NULL;

/* local function declarations  */


// states
void system_controller_state_init(void);
void system_controller_state_final(void);
void system_controller_state_init_communicastion_interfaces(void);
void system_controller_state_load_settings(void);
void system_controller_state_waiting_for_user_interrupt(void);
void system_controller_state_run_configuration_terminal(void);
void system_controller_state_init_pid_controller(void);
void system_controller_state_init_timer(void);
void system_controller_state_run_pid_controller(void);
void system_controller_state_null(void);


/* *** FUNCTION DEFINITIONS ************************************************** */
void system_controller_state_state_machine(void)
{

	while(state_machine_running) {
		switch(current_state) {

			case STATE_INIT_COMMUNICATION_INTERFACES:
				system_controller_state_init_communicastion_interfaces();
			break;

			case STATE_LOAD_SETTINGS:
				system_controller_state_load_settings();
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

			case STATE_INIT_TIMER:
				system_controller_state_init_timer();
			break;

			case STATE_RUN_PID_CONTROLLER:
				system_controller_state_run_pid_controller();
			break;

			case STATE_INIT:
				system_controller_state_init();
			break;

			case STATE_FINAL:
				system_controller_state_final();
			break;

			case STATE_NULL:
				system_controller_state_null();
			break;
		}

		//change state
		current_state = next_state;
	}
}

void system_controller_state_null(void)
{
	printf("system_controller_state_null(void)\n");

	/* *** ENTRY *** */

	/* **** DO ***** */

	state_machine_running = false;

	/* *** EXIT **** */

	next_state = STATE_NULL;
}


void system_controller_state_init(void)
{
	/* *** ENTRY *** */

	/* **** DO ***** */

	/* *** EXIT **** */

	next_state = STATE_NULL;
}


void system_controller_state_final(void)
{
	/* *** ENTRY *** */

	/* **** DO ***** */

	/* *** EXIT **** */

	next_state = STATE_NULL;
}


void system_controller_state_init_communicastion_interfaces(void)
{
	/* *** ENTRY *** */


	/* **** DO ***** */
	UART_init(38400);	/* Init UART mit 9600 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */

	/* *** EXIT **** */

	next_state = STATE_LOAD_SETTINGS;

	vt100_clear_all();
	printf("system_controller_state_init_communicastion_interfaces(void)\n");
}


void system_controller_state_load_settings(void)
{
	printf("system_controller_state_load_settings(void)\n");

	/* *** ENTRY *** */

	uint8_t i;
	uint8_t valid_settings = 0;

	/* **** DO ***** */


	//Init default-setting


	configuration_setting_default.pid_scalingfactor = 1;				//TODO: #define defaultsettings somewhere
	configuration_setting_default.position_multiplier = 1;
	configuration_setting_default.setting_version = CONFIGURATION_SETTING_VERSION;
	strcpy(configuration_setting_default.comment, "- new -");

	//read index of current setting
	configuration_setting_current_index = eeprom_read_byte(&configuration_setting_current_index_eeprom);

	//and validate
	if(configuration_setting_current_index >= CONFIGURATION_SETTING_COUNT) {
		configuration_setting_current_index = 0;
	}

	//read settings from eeprom
	for(i = 0;i < CONFIGURATION_SETTING_COUNT;i++) {

		//read setting from eeprom into temp variable
		eeprom_read_block(&configuration_setting_data[i],
						  &configuration_setting_data_eeprom[i],
						  sizeof(configuration_setting_t));

		//validate; if read setting is valid; write into array
		//TODO: implemnt a more spohisticated validation against the real versionnumber
		if(configuration_setting_data[i].setting_version == CONFIGURATION_SETTING_VERSION) {
			valid_settings++;
		} else {
			//Copy default
			memcpy(&configuration_setting_data[i],
				   &configuration_setting_default,
				   sizeof(configuration_setting_t));
		}
	}

	/* *** EXIT **** */

	//if no valid configuration found, then run config terminal directly
	if(valid_settings > 0) {
		next_state = STATE_WAITING_FOR_USER_INTERRUPT;
	} else {
		next_state = STATE_RUN_CONFIGURATION_TERMINAL;
	}
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
		next_state = STATE_RUN_CONFIGURATION_TERMINAL;
	} else {
		next_state = STATE_INIT_PID_CONTROLLER;
	}
}


void system_controller_state_run_configuration_terminal(void)
{
	/* *** ENTRY *** */

	printf("system_controller_state_run_configuration_terminal(void)\n");

	/* **** DO ***** */

	/* *** EXIT **** */

	next_state = STATE_INIT_PID_CONTROLLER;
}


void system_controller_state_init_pid_controller(void)
{
	/* *** ENTRY *** */

	printf("system_controller_state_init_pid_controller(void)\n");

	/* **** DO ***** */

	/* *** EXIT **** */

	next_state = STATE_INIT_TIMER;
}


void system_controller_state_init_timer(void)
{
	/* *** ENTRY *** */

	printf("system_controller_state_init_timer(void)\n");

	/* **** DO ***** */

	/* *** EXIT **** */

	next_state = STATE_RUN_PID_CONTROLLER;
}


void system_controller_state_run_pid_controller(void)
{
	/* *** ENTRY *** */

	printf("system_controller_state_run_pid_controller(void)\n");

	/* **** DO ***** */

	/* *** EXIT **** */

	next_state = STATE_NULL;
}
