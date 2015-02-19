/*
 * vt100.c
 *
 *  Created on: Dec 5, 2013
 *      Author: alexandermertens
 *
 *      �bersicht g�ngiger VT100 Befehle
 *      http://www-user.tu-chemnitz.de/~heha/hs/terminal/terminal.htm
 *      http://ascii-table.com/ansi-escape-sequences-vt-100.php
 *      http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=80382&start=0
 *
 */


/* *** INCLUDES ************************************************************** */
#include "vt100.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

#define ASCII_ESC		(char)(27)
#define ASCII_LF		(char)(10)

char vt100_input_buffer[VT100_INPUT_BUFFER_SIZE];

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */


void vt100_clear_screen(void)
{
	char cmd[] = {ASCII_ESC, '[', '2', 'J', '\0'};

	printf(cmd);
}

void vt100_clear_input_buffer(void)
{

	//den eventuell noch gefüllten Sendebuffer des PC lesen und verwerfen
	while(strlen(vt100_input_buffer) >= VT100_INPUT_BUFFER_SIZE - 1) {
		fgets(vt100_input_buffer, VT100_INPUT_BUFFER_SIZE, stdin);
	}

	//flush is alway a good idea!
	fflush(stdin);

	//Buffer als "leer" markieren
	vt100_input_buffer[0] = '\0';
}

void vt100_clear_all(void)
{
	vt100_clear_screen();
	vt100_set_cursor_top_left();
	vt100_clear_input_buffer();
	printf("\n");
	printf("\n");
}

void vt100_set_cursor_top_left(void)
{
	char cmd[] = {ASCII_ESC, '[', 'H', '\0'};

	printf(cmd);
}

void vt100_set_cursor_on_position(uint8_t line, uint8_t column)
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


char vt100_get_choice(void)
{
	char choice = 0;

	//Eingabe einlesen
	fgets(vt100_input_buffer, VT100_INPUT_BUFFER_SIZE, stdin);

	//Nur den ersten Char verarbeiten
	choice = vt100_input_buffer[0];

	//Wertebereichprüfung und Kconvertierung zu Großbuchstaben
	if(isalpha(choice)) {
		choice = toupper(choice);
	} else {
		choice = 0;
	}

	//Buffer leeren
	vt100_clear_input_buffer();

	return choice;
}


void vt100_get_string(char *str, uint8_t max_len)
{
	char *c_ptr;

	vt100_clear_input_buffer();
	fgets(vt100_input_buffer, max_len, stdin);

	if(strlen(vt100_input_buffer) > 0) {
		strcpy(str, vt100_input_buffer);
		c_ptr = strchr(str,'\n');

		if(c_ptr != NULL) {
			*c_ptr = '\0';
		}
	}

	vt100_clear_input_buffer();
}

uint16_t vt100_get_integer(const uint16_t current_value,
						   const uint16_t min_range,
						   const uint16_t max_range)
{
	bool successful = false;
	uint16_t new_value = 0;

	do {
		printf("Please enter a integer between %u and %u:\n", min_range, max_range);

		//read user input
		vt100_clear_input_buffer();
		fgets(vt100_input_buffer, VT100_INPUT_BUFFER_SIZE, stdin);

		//check if nothing is entered. so don't change the old value
		if(vt100_input_buffer[0] != ASCII_LF) {
			//if strlen > nothing
			//try to convert to uint16_t;
			//check should be positive (here: == 1) if successful
			if(sscanf(vt100_input_buffer, "%u",&new_value)) {

				//check, if new value is in range
				//and is not negative (uint!!) (input string begins with a '-')
				if(new_value >= min_range &&
				   new_value <= max_range &&
				   vt100_input_buffer[0] != '-')
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
