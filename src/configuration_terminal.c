/*
 * configuration_terminal.c
 *
 *  Created on: 28.11.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "configuration_terminal.h"
#include "lib/uart.h"

#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* *** DECLARATIONS ********************************************************** */

//-------------------Defines--------------------//

#define INPUT_BUFFER_SIZE 8

/* local type and constants     */
typedef enum {
	STATE_MAIN_MENU,
	STATE_SETPID,
	STATE_TARA_ACCEL
} configuration_terminal_state_t;

static configuration_terminal_state_t current_state = STATE_MAIN_MENU;

static char input_buffer[INPUT_BUFFER_SIZE];



/* local function declarations  */
void configuration_terminal_clear(void);
char configuration_terminal_get_choice(void);

/* states */
void configuration_terminal_state_main_menu(void);
void configuration_terminal_state_setPID(void);
void configuration_terminal_state_tata_acceleration(void);

/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{

	for(;;) {

		switch(current_state) {

			case STATE_MAIN_MENU:
				configuration_terminal_state_main_menu();
			break;

			case STATE_SETPID:
				configuration_terminal_state_setPID();
			break;

			case STATE_TARA_ACCEL:
				configuration_terminal_state_tata_acceleration();
			break;
		}

		_delay_ms(2000.0);
	}
}


void configuration_terminal_state_main_menu(void)
{


	//ENTRY
	configuration_terminal_clear();

	printf("MAIN MENU\n\n[H]odensack:\n[I]n kalte\n[K]acke");

	//DO

	for(;;)
	{
		while(!UART_char_received()) {
			printf(".");
			_delay_ms(1000.0);
		}
		UART_clr_rx_buf();

		printf("\nLoop interrupted\nPlease type char:\n");

		printf("input: %c\n", configuration_terminal_get_choice());

	}
	//EXIT
	current_state = STATE_SETPID;
}

void configuration_terminal_state_setPID(void)
{
	configuration_terminal_clear();

	printf("MENU SET PID\n");

	current_state = STATE_TARA_ACCEL;
}

void configuration_terminal_state_tata_acceleration(void)
{
	configuration_terminal_clear();

	printf("MENU TARA ACCELERATIONSENSOR\n");

	current_state = STATE_MAIN_MENU;

}

void configuration_terminal_clear(void)
{
	/* http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=80382&start=0 */
	char cmd[] = {27, '[', '2', 'J', '\0'};

	printf(cmd);
}

char configuration_terminal_get_choice(void)
{
	char choice = 0;

	fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);

	choice = input_buffer[0];

	if(!isalpha(choice)) {
		choice = 0;
	}

	//Überlangen string lesen und verwerfen
	while(strlen(input_buffer) >= INPUT_BUFFER_SIZE - 1) {
		fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
	}

	return choice;
}


