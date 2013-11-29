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
#define ASCII_ESC		  (char)(27)

/* local type and constants     */
typedef enum {
	STATE_MAIN_MENU,
	STATE_SETPID,
	STATE_TARA_ACCEL
} configuration_terminal_state_t;

static configuration_terminal_state_t current_state = STATE_MAIN_MENU;

static char input_buffer[INPUT_BUFFER_SIZE];



/* local function declarations  */
void configuration_terminal_clear_screen(void);
void configuration_terminal_set_cursor_top_left(void);
char configuration_terminal_get_choice(void);
void configuration_terminal_set_cursor_on_position(uint8_t, uint8_t); /* uint8_t x, uint8_t y */

void configuration_terminal_clear_input_buffer(void);

/* states */
void configuration_terminal_state_main_menu(void);
void configuration_terminal_state_setPID(void);
void configuration_terminal_state_tata_acceleration(void);

/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{

	configuration_terminal_clear_screen();

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
	configuration_terminal_clear_screen();
	configuration_terminal_set_cursor_top_left();
	configuration_terminal_clear_input_buffer();

	printf("MAIN MENU\n\n[H]odensack:\n[I]n kalte\n[K]acke");

	//DO


	while(!UART_char_received()) {
		printf(".");
		_delay_ms(1000.0);
	}
	UART_clr_rx_buf();

	printf("\nLoop interrupted\nPlease type char:\n");

	printf("input: %c\n", configuration_terminal_get_choice());


	//EXIT
	current_state = STATE_SETPID;
}

void configuration_terminal_state_setPID(void)
{
	configuration_terminal_clear_screen();

	printf("MENU SET PID\n");

	current_state = STATE_TARA_ACCEL;
}

void configuration_terminal_state_tata_acceleration(void)
{
	configuration_terminal_clear_screen();
	configuration_terminal_set_cursor_on_position(10, 5);

	printf("MENU TARA ACCELERATIONSENSOR\n");

	_delay_ms(3000.0);

	current_state = STATE_MAIN_MENU;

}

char configuration_terminal_get_choice(void)
{
	char choice = 0;

	//Eingabe einlesen
	fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);

	//Nur den ersten Char verarbeiten
	choice = input_buffer[0];

	//Wertebereichprüfung und convertierung zu Großbuchstaben
	if(isalpha(choice)) {
		choice = toupper(choice);
	} else {
		choice = 0;
	}

	//Buffer leeren
	configuration_terminal_clear_input_buffer();

	return choice;
}

void configuration_terminal_clear_input_buffer(void)
{
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
	char cmd[50];

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

