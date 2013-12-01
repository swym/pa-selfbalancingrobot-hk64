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
#include <stdbool.h>
#include <ctype.h>

#include "pid.h"
#include "acceleration_t.h"

/* *** DECLARATIONS ********************************************************** */

//-------------------Defines--------------------//

#define INPUT_BUFFER_SIZE		24

#define ASCII_ESC		  		(char)(27)
#define ASCII_LF				(char)(10)

#define STATE_WAITING_TIMEOUT	5			//Seconds for Timeout
#define STATE_WAITING_PARTS   	4

/* local type and constants     */
typedef enum {
	STATE_WAITING,
	STATE_MAIN_MENU,
	STATE_PID_MENU,
	STATE_PID_SET_P,
	STATE_PID_SET_I,
	STATE_PID_SET_D,
	STATE_PID_SET_SCALINGFACTOR,
	STATE_ACCELERATIONSENSOR_MENU,
	STATE_ACCELERATIONSENSOR_SHOW,
	STATE_ACCELERATIONSENSOR_SET_ZERO,
	STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR,
	STATE_FINAL,
	STATE_NULL
} configuration_terminal_state_t;

static configuration_terminal_state_t current_state = STATE_WAITING;
static configuration_terminal_state_t next_state = STATE_NULL;

static pidData_t current_pid_settings;
static uint8_t pid_scaling_factor;			//TODO: only for dev; put in pidData struct
static uint16_t accel_pos_scaling_factor;   //TODO: only for dev; put in fitting struct
static acceleration_t acceleration_offset;	//TODO: only for dev;


static char input_buffer[INPUT_BUFFER_SIZE];


/* local function declarations  */
void configuration_terminal_clear_screen(void);
void configuration_terminal_set_cursor_top_left(void);
char configuration_terminal_get_choice(void);
uint16_t configuration_terminal_get_integer(const uint16_t current_value,
											const uint16_t min_range,
											const uint16_t max_range);
void configuration_terminal_clear_all(void);
void configuration_terminal_set_cursor_on_position(uint8_t, uint8_t); /* uint8_t x, uint8_t y */
void configuration_terminal_clear_input_buffer(void);

/* states */
void configuration_terminal_state_waiting(void);
void configuration_terminal_state_main_menu(void);
void configuration_terminal_state_PID_menu(void);
void configuration_terminal_state_PID_set_P(void);
void configuration_terminal_state_PID_set_I(void);
void configuration_terminal_state_PID_set_D(void);
void configuration_terminal_state_PID_set_scalingfactor(void);
void configuration_terminal_state_accelerationsensor_menu(void);
void configuration_terminal_state_accelerationsensor_show(void);
void configuration_terminal_state_accelerationsensor_set_zero(void);
void configuration_terminal_state_accelerationsensor_set_scalingfactor(void);

/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{

	//TODO: should moved to a better place
	configuration_terminal_clear_screen();
/*
	current_pid_settings.P_Factor = 10;
	current_pid_settings.I_Factor = 10;
	current_pid_settings.D_Factor = 10;
	current_pid_settings.lastProcessValue = 0;
	current_pid_settings.maxError = 0;
	current_pid_settings.maxSumError = 0;
	current_pid_settings.sumError = 0;
*/
 	while(current_state != STATE_FINAL) {
		switch(current_state) {

			case STATE_WAITING:
				configuration_terminal_state_waiting();
			break;

			case STATE_MAIN_MENU:
				configuration_terminal_state_main_menu();
			break;

			case STATE_PID_MENU:
				configuration_terminal_state_PID_menu();
			break;

			case STATE_PID_SET_P:
				configuration_terminal_state_PID_set_P();
			break;

			case STATE_PID_SET_I:
				configuration_terminal_state_PID_set_I();
			break;

			case STATE_PID_SET_D:
				configuration_terminal_state_PID_set_D();
			break;

			case STATE_PID_SET_SCALINGFACTOR:
				configuration_terminal_state_PID_set_scalingfactor();
			break;

			case STATE_ACCELERATIONSENSOR_MENU:
				configuration_terminal_state_accelerationsensor_menu();
			break;

			case STATE_ACCELERATIONSENSOR_SHOW:
				configuration_terminal_state_accelerationsensor_show();
			break;

			case STATE_ACCELERATIONSENSOR_SET_ZERO:
				configuration_terminal_state_accelerationsensor_set_zero();
			break;

			case STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR:
				configuration_terminal_state_accelerationsensor_set_scalingfactor();
			break;

			case STATE_FINAL:
			break;
			case STATE_NULL:
			break;
		}

		current_state = next_state;
		next_state = STATE_NULL;
	}
 	configuration_terminal_clear_all();
 	printf("FINAL");
}


void configuration_terminal_state_waiting(void)
{
	uint8_t waiting_time = STATE_WAITING_TIMEOUT;
	bool user_irq_received = false;
	double delay = 1000.0/(STATE_WAITING_PARTS + 1);

	uint8_t parts_of_seconds_counter = 0;


	// ENTRY
	configuration_terminal_clear_all();

	printf("Press any key for entering configuration menu...\n");

	// DO
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
			parts_of_seconds_counter = STATE_WAITING_PARTS;
		} else {
			printf(".");
			parts_of_seconds_counter--;
		}

		_delay_ms(delay);
	}

	if(user_irq_received) {
		next_state = STATE_MAIN_MENU;
	} else {
		next_state = STATE_FINAL;
	}

	// EXIT

}

void configuration_terminal_state_main_menu(void)
{
	char choice;

	// ENTRY
	configuration_terminal_clear_all();

	//Greeting
	printf("   === MAIN MENU ===\n\n");
	printf("Current PID Parameters:\n  P: %u\n  I: %u\n  D: %u\n\n",
									current_pid_settings.P_Factor,
									current_pid_settings.I_Factor,
									current_pid_settings.D_Factor);
	printf("Select a option by pressing the correspondending key\n\n");
	printf(" [P] Configure PID-controller\n");
	printf(" [A] Configure accelerationsensor\n\n");
	printf(" [X] Start PID controller with current parameters\n\n");

	// DO
	//Waiting for users choice
	do {
		choice = configuration_terminal_get_choice();

		if(choice == 'P') {
			next_state = STATE_PID_MENU;
		} else if(choice == 'A') {
			next_state = STATE_ACCELERATIONSENSOR_MENU;
		} else if(choice == 'X') {
			next_state = STATE_FINAL;
		} else {
			printf("Invalid choice! Please retry:\n");
			choice = 0;
		}

	} while(choice == 0);

	// EXIT

}


void configuration_terminal_state_PID_menu(void)
{
	char choice;

	// ENTRY
	configuration_terminal_clear_all();

	//Greeting
	printf("   === PID CONTROLLER MENU ===\n\n");
	printf("Current PID Parameters:\n  P: %u\n  I: %u\n  D: %u\n\n",
									current_pid_settings.P_Factor,
									current_pid_settings.I_Factor,
									current_pid_settings.D_Factor);
	printf("Select a option by pressing the correspondending key\n\n");
	printf(" [P] change proportional parameter\n");
	printf(" [I] change integral parameter\n");
	printf(" [D] change derivative parameter\n\n");
	printf(" [F] change scaling factor\n\n");
	printf(" [X] Back to Main Menu\n");

	// DO
	do {
		choice = configuration_terminal_get_choice();

		if(choice == 'P') {
			next_state = STATE_PID_SET_P;
		} else if(choice == 'I') {
			next_state = STATE_PID_SET_I;
		} else if(choice == 'D') {
			next_state = STATE_PID_SET_D;
		} else if(choice == 'F') {
			next_state = STATE_PID_SET_SCALINGFACTOR;
		} else if(choice == 'X') {
			next_state = STATE_MAIN_MENU;
		} else {
			printf("Invalid choice! Please retry:\n");
			choice = 0;
		}

	} while(choice == 0);

	// EXIT

}


void configuration_terminal_state_PID_set_P(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== CHANGE PROPORTIONAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", current_pid_settings.P_Factor);


	// DO
	current_pid_settings.P_Factor = configuration_terminal_get_integer(
										current_pid_settings.P_Factor,
										0,
										UINT16_MAX);
	next_state = STATE_PID_MENU;

	// EXIT

}


void configuration_terminal_state_PID_set_I(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== CHANGE INGETRAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", current_pid_settings.I_Factor);

	// DO
	current_pid_settings.I_Factor = configuration_terminal_get_integer(
										current_pid_settings.I_Factor,
										0,
										UINT16_MAX);
	next_state = STATE_PID_MENU;
	// EXIT

}


void configuration_terminal_state_PID_set_D(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== CHANGE PROPORTIONAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", current_pid_settings.D_Factor);


	// DO
	current_pid_settings.D_Factor = configuration_terminal_get_integer(
										current_pid_settings.D_Factor,
										0,
										UINT16_MAX);
	next_state = STATE_PID_MENU;


	// EXIT

}


void configuration_terminal_state_PID_set_scalingfactor(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== PID SCALING FACTOR ===\n\n");
	printf("Current value: %u\n\n", pid_scaling_factor);

	// DO
	pid_scaling_factor = configuration_terminal_get_integer(
							pid_scaling_factor, 1, UINT8_MAX);

	next_state = STATE_PID_MENU;


	// EXIT

}

void configuration_terminal_state_accelerationsensor_menu(void)
{
	char choice;

	// ENTRY
	configuration_terminal_clear_all();

	//Greeting
	printf("   === ACCELERATIONSENSOR MENU ===\n\n");
	printf("Select a option by pressing the correspondending key\n\n");
	printf(" [Z] Set zero\n");
	printf(" [S] Show acceleration and position\n");
	printf(" [F] Change scaling factor\n\n");
	printf(" [X] Back to Main Menu\n");

	// DO
	do {
		choice = configuration_terminal_get_choice();

		if(choice == 'Z') {
			next_state = STATE_ACCELERATIONSENSOR_SET_ZERO;
		} else if(choice == 'S') {
			next_state = STATE_ACCELERATIONSENSOR_SHOW;
		} else if(choice == 'F') {
			next_state = STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR;
		} else if(choice == 'X') {
			next_state = STATE_MAIN_MENU;
		} else {
			printf("Invalid choice! Please retry:\n");
			choice = 0;
		}

	} while(choice == 0);


	// EXIT

}


void configuration_terminal_state_accelerationsensor_show(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("ACCEL SHOW MENU\n\n");


	printf("X:%d\nY:%d\nZ:%d\n",	acceleration_offset.x,
									acceleration_offset.y,
									acceleration_offset.z);

	// DO

	_delay_ms(2000.0);


	next_state = STATE_ACCELERATIONSENSOR_MENU;


	// EXIT

}


void configuration_terminal_state_accelerationsensor_set_zero(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("ACCEL SET ZERO MENU");


	// DO
	//TODO: dummy implementation for dev
	acceleration_offset.x += 100;
	acceleration_offset.y += 100;
	acceleration_offset.z += 100;


	printf("Current Position is now new zero!");
	_delay_ms(2000.0);

	next_state = STATE_ACCELERATIONSENSOR_MENU;
	// EXIT

}


void configuration_terminal_state_accelerationsensor_set_scalingfactor(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== ACCELERATIONSENSOR SCALING FACTOR ===\n\n");
	printf("Current value: %u\n\n", accel_pos_scaling_factor);

	// DO
	accel_pos_scaling_factor = configuration_terminal_get_integer(
									accel_pos_scaling_factor, 1, UINT16_MAX);

	next_state = STATE_ACCELERATIONSENSOR_MENU;

	// EXIT

}


/* *** HELPERFUNCTIONS *** */

char configuration_terminal_get_choice(void)
{
	char choice = 0;

	//Eingabe einlesen
	fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);

	//Nur den ersten Char verarbeiten
	choice = input_buffer[0];

	//Wertebereichprüfung und Kconvertierung zu Großbuchstaben
	if(isalpha(choice)) {
		choice = toupper(choice);
	} else {
		choice = 0;
	}

	//Buffer leeren
	configuration_terminal_clear_input_buffer();

	return choice;
}


uint16_t configuration_terminal_get_integer(const uint16_t current_value,
											const uint16_t min_range,
											const uint16_t max_range)
{
	bool successful = false;
	uint16_t new_value = 0;

	do {
		printf("Please enter a integer between %u and %u:\n", min_range, max_range);

		//read user input
		configuration_terminal_clear_input_buffer();
		fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);

		//check if nothing is entered. so don't change the old value
		if(input_buffer[0] != ASCII_LF) {
			//if strlen > nothing
			//try to convert to uint16_t;
			//check should be positive (here: == 1) if successful
			if(sscanf(input_buffer, "%u",&new_value)) {

				//check, if new value is in range
				//and is not negative (uint!!) (input string begins with a '-')
				if(new_value >= min_range &&
				   new_value <= max_range &&
				   input_buffer[0] != '-')
				{
					//if conversion is successful; break loop
					successful = true;
				} else {
					//loop again
					printf("New value is out of range! Please retry:\n");
					successful = false;
				}
			} else {
				//loop adain
				printf("Input is not valid! Please retry:\n");
				successful = false;
			}
		} else {
			//break loop but return current value
			printf("Nothing entered! Old value will be used!\n");
			new_value = current_value;
			successful = true;
		}
	} while(!successful);

	return new_value;
}

void configuration_terminal_clear_all(void)
{
	configuration_terminal_clear_screen();
	configuration_terminal_set_cursor_top_left();
	configuration_terminal_clear_input_buffer();
}

void configuration_terminal_clear_input_buffer(void)
{
	//TODO: fflush() verwenden

	//den eventuell noch gefüllten Sendebuffer des PC lesen und verwerfen
	while(strlen(input_buffer) >= INPUT_BUFFER_SIZE - 1) {
		fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
	}

	//Buffer als "leer" markieren
	input_buffer[0] = '\0';
}


void configuration_terminal_clear_screen(void)
{
	/* http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=80382&start=0 */
	char cmd[] = {ASCII_ESC, '[', '2', 'J', '\0'};

	printf(cmd);
}

void configuration_terminal_set_cursor_top_left(void)
{
	/* http://www-user.tu-chemnitz.de/~heha/hs/terminal/terminal.htm */
	/* http://ascii-table.com/ansi-escape-sequences-vt-100.php */
	char cmd[] = {ASCII_ESC, '[', 'H', '\0'};

	printf(cmd);
}

void configuration_terminal_set_cursor_on_position(uint8_t line, uint8_t column)
{
	char cmd[24];

	//check range of line; set to 1 if invalid
	if(line < 1) {
		line = 1;
	}

	//check range of line; set to 1 if invalid
	if(column < 1) {
		column = 1;
	}
	sprintf(cmd, "%c[%d;%dH", ASCII_ESC, line, column);
	printf(cmd);
}

