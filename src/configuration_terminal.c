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
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "pid.h"
#include "acceleration_t.h"

/* *** DECLARATIONS ********************************************************** */

//-------------------Defines--------------------//

#define INPUT_BUFFER_SIZE		80

#define ASCII_ESC		  		(char)(27)
#define ASCII_LF				(char)(10)

#define STATE_WAITING_TIMEOUT	5			//Seconds for Timeout
#define STATE_WAITING_PARTS   	4

#define SETTINGS_COUNT							6


#define SETTING_VERSION_PID						1
#define SETTING_VERSION_ACCELERATION_OFFSET		1
#define CONFIGURATION_SETTING_COMMENT_LENGTH	40

/* local type and constants     */
typedef enum {
	STATE_WAITING,
	STATE_LOADING_SETTINGS,
	STATE_SELECT_SETTINGS,
	STATE_WRITE_SETTINGS,
	STATE_EXPORT_SETTINGS,
	STATE_MAIN_MENU,
	STATE_PID_SET_P,
	STATE_PID_SET_I,
	STATE_PID_SET_D,
	STATE_PID_SET_SCALINGFACTOR,
	STATE_ACCELERATIONSENSOR_SET_ZERO,
	STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR,
	STATE_FINAL,
	STATE_NULL
} configuration_terminal_state_t;

typedef struct {
	pidData_t pid_setting;
	uint8_t pid_setting_version;
	acceleration_t acceleration_offset;
	uint8_t acceleration_offset_version;
	uint8_t position_multiplier;
	uint8_t pid_factor;
	char comment[CONFIGURATION_SETTING_COMMENT_LENGTH];
} configuration_setting_t;

static configuration_terminal_state_t current_state = STATE_LOADING_SETTINGS;
static configuration_terminal_state_t next_state = STATE_NULL;

static configuration_setting_t default_setting;
static configuration_setting_t settings[SETTINGS_COUNT];
static uint8_t current_setting = 0;

configuration_setting_t ee_settings[SETTINGS_COUNT] __attribute__ ((section (".eeprom")));
uint8_t ee_current_setting __attribute__ ((section (".eeprom")));

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
void configuration_terminal_state_loading_settings(void);
void configuration_terminal_state_waiting(void);
void configuration_terminal_state_select_settings(void);
void configuration_terminal_state_write_settings(void);
void configuration_terminal_state_export_settings(void);
void configuration_terminal_state_main_menu(void);
void configuration_terminal_state_PID_set_P(void);
void configuration_terminal_state_PID_set_I(void);
void configuration_terminal_state_PID_set_D(void);
void configuration_terminal_state_PID_set_scalingfactor(void);
void configuration_terminal_state_accelerationsensor_set_zero(void);
void configuration_terminal_state_accelerationsensor_set_scalingfactor(void);

/* *** FUNCTION DEFINITIONS ************************************************** */


void configuration_terminal_state_machine(void)
{
	default_setting.pid_factor = 1;				//TODO: #define defaultsettings somewhere
	default_setting.position_multiplier = 1;
	default_setting.acceleration_offset_version = SETTING_VERSION_ACCELERATION_OFFSET;
	default_setting.pid_setting_version = SETTING_VERSION_PID;
	strcpy(default_setting.comment, "- new -");

	//TODO: should moved to a better place
	configuration_terminal_clear_screen();

 	while(current_state != STATE_FINAL) {
		switch(current_state) {

			case STATE_LOADING_SETTINGS:
				configuration_terminal_state_loading_settings();
			break;

			case STATE_WAITING:
				configuration_terminal_state_waiting();
			break;

			case STATE_SELECT_SETTINGS:
				configuration_terminal_state_select_settings();
			break;

			case STATE_WRITE_SETTINGS:
				configuration_terminal_state_write_settings();
			break;

			case STATE_EXPORT_SETTINGS:
				configuration_terminal_state_export_settings();
			break;

			case STATE_MAIN_MENU:
				configuration_terminal_state_main_menu();
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

			case STATE_ACCELERATIONSENSOR_SET_ZERO:
				configuration_terminal_state_accelerationsensor_set_zero();
			break;

			case STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR:
				configuration_terminal_state_accelerationsensor_set_scalingfactor();
			break;

			case STATE_FINAL:
				//TODO: run system with current settings
			break;
			case STATE_NULL:
				//TODO: Implement Null state
			break;
		}

		current_state = next_state;
		next_state = STATE_NULL;
	}
 	configuration_terminal_clear_all();
 	printf("FINAL");
}

void configuration_terminal_state_loading_settings(void)
{
	uint8_t i;
	uint8_t valid_settings;

	//read index of current setting
	current_setting = eeprom_read_byte(&ee_current_setting);

	//and validate
	if(current_setting >= SETTINGS_COUNT) {
		current_setting = 0;
	}

	//read settings from eeprom
	for(i = 0;i < SETTINGS_COUNT;i++) {

		//read setting from eeprom into temp variable
		eeprom_read_block(&settings[i],
						  &ee_settings[i],
						  sizeof(configuration_setting_t));

		//validate; if read setting is valid; write into array
		//TODO: implemnt a more spohisticated validation against the real versionnumber
		if(settings[i].acceleration_offset_version == SETTING_VERSION_ACCELERATION_OFFSET &&
		   settings[i].pid_setting_version == SETTING_VERSION_PID) {
			valid_settings++;
		} else {
			//Copy default
			memcpy(&settings[i], &default_setting, sizeof(configuration_setting_t));
		}
	}

	next_state = STATE_WAITING;
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

void configuration_terminal_state_select_settings(void)
{
	//ENTRY
	uint8_t i = 0;

	configuration_terminal_clear_all();


	printf("           P      I      D      F      M     Ox     Oy     Oz\n\n");


	for(i = 0;i < SETTINGS_COUNT;i++) {
		printf(" [%u] - \"%s\"\n", i, settings[i].comment);
		printf("       %5u  %5u  %5u  %5u  %5u  %5u  %5u  %5u\n\n",
				settings[i].pid_setting.P_Factor,
				settings[i].pid_setting.I_Factor,
				settings[i].pid_setting.D_Factor,
				settings[i].pid_factor,
				settings[i].position_multiplier,
				settings[i].acceleration_offset.x,
				settings[i].acceleration_offset.y,
				settings[i].acceleration_offset.z);
	}

	printf("Index of current setting is: %u\n", current_setting);

	//DO

	current_setting = configuration_terminal_get_integer(
						current_setting,
						0,
						SETTINGS_COUNT);


	//EXIT

	next_state = STATE_MAIN_MENU;
}

void configuration_terminal_state_write_settings(void)
{
	uint8_t i;


	//ENTRY
	configuration_terminal_clear_all();

	//DO
	printf("Writing settings to eeprom....\n");

	for(i = 0;i < SETTINGS_COUNT;i++) {

		eeprom_write_block(&settings[i],
						   &ee_settings[i],
						   sizeof(configuration_setting_t));
	}

	eeprom_write_byte(&ee_current_setting, current_setting);

	//EXIT
	next_state = STATE_MAIN_MENU;
}

void configuration_terminal_state_export_settings(void)
{
	uint8_t i;

	//ENTRY
	configuration_terminal_clear_all();

	//DO
	for(i = 0;i < SETTINGS_COUNT;i++) {
		printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,\"%s\"\n",
				i,
				settings[i].pid_setting.P_Factor,
				settings[i].pid_setting.I_Factor,
				settings[i].pid_setting.D_Factor,
				settings[i].pid_factor,
				settings[i].position_multiplier,
				settings[i].acceleration_offset.x,
				settings[i].acceleration_offset.y,
				settings[i].acceleration_offset.z,
				settings[i].comment);
	}

	printf("\n\nPress any key to go back to main menu.\n");

	//wait for user input; but don't use it
	configuration_terminal_get_choice();

	//EXIT
	next_state = STATE_MAIN_MENU;
}

void configuration_terminal_state_main_menu(void)
{
	char choice;

	// ENTRY
	configuration_terminal_clear_all();

	//Greeting

	printf("                          [Current Configuration]\n\n");
	printf("[C] - \"%s\"\n\n", settings[current_setting].comment);


	printf("    PID Controller                          Accelerationsensor\n\n");
	printf("[P] - Proportional Parameter : %5u    [M] - Position Multiplier    : %5u\n",
					settings[current_setting].pid_setting.P_Factor,
					settings[current_setting].position_multiplier);

	printf("[I] - Integral Parameter     : %5u    [O] - Set Offset           X : %5u\n",
					settings[current_setting].pid_setting.I_Factor,
					settings[current_setting].acceleration_offset.x);

	printf("[D] - Derivative Parameter   : %5u                               Y : %5u\n",
					settings[current_setting].pid_setting.D_Factor,
					settings[current_setting].acceleration_offset.y);

	printf("[F] - Factor                 : %5u                               Z : %5u\n",
					settings[current_setting].pid_factor,
					settings[current_setting].acceleration_offset.z);

	printf("\n\n    Settings\n\n[S] - Select current setting\n[W] - Write settings to EEPROM\n[E] - Export to csv\n");
	printf("\n\n    Run\n\n[R] - Run system with current configuration\n");

	// DO
	//Waiting for users choice
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
		} else if(choice == 'M') {
			next_state = STATE_ACCELERATIONSENSOR_SET_SCALINGFACTOR;
		} else if(choice == 'O') {
			next_state = STATE_ACCELERATIONSENSOR_SET_ZERO;
		} else if(choice == 'S') {
			next_state = STATE_SELECT_SETTINGS;
		} else if(choice == 'W') {
			next_state = STATE_WRITE_SETTINGS;
		} else if(choice == 'E') {
			next_state = STATE_EXPORT_SETTINGS;
		} else if(choice == 'R') {
			next_state = STATE_FINAL;
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
	printf("Current value: %u\n\n", settings[current_setting].pid_setting.P_Factor);


	// DO
	settings[current_setting].pid_setting.P_Factor =
			configuration_terminal_get_integer(settings[current_setting].pid_setting.P_Factor,
											   0,
											   UINT16_MAX);
	next_state = STATE_MAIN_MENU;

	// EXIT

}


void configuration_terminal_state_PID_set_I(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== CHANGE INGETRAL PARAMETER ===\n\n");
	printf("Current value: %u\n\n", settings[current_setting].pid_setting.I_Factor);


	// DO
	settings[current_setting].pid_setting.I_Factor =
			configuration_terminal_get_integer(settings[current_setting].pid_setting.I_Factor,
											   0,
											   UINT16_MAX);
	next_state = STATE_MAIN_MENU;
	// EXIT

}


void configuration_terminal_state_PID_set_D(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== CHANGE DERIVATIVE PARAMETER ===\n\n");
	printf("Current value: %u\n\n", settings[current_setting].pid_setting.D_Factor);


	// DO
	settings[current_setting].pid_setting.D_Factor =
			configuration_terminal_get_integer(settings[current_setting].pid_setting.D_Factor,
											   0,
											   UINT16_MAX);
	next_state = STATE_MAIN_MENU;


	// EXIT

}


void configuration_terminal_state_PID_set_scalingfactor(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("===CHANGE PID SCALING FACTOR ===\n\n");
//	printf("Current value: %u\n\n", pid_scaling_factor);

	// DO
	settings[current_setting].pid_factor = (uint8_t)configuration_terminal_get_integer(
			settings[current_setting].pid_factor, 1, UINT8_MAX);



	next_state = STATE_MAIN_MENU;


	// EXIT

}


void configuration_terminal_state_accelerationsensor_set_zero(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("ACCEL SET ZERO MENU\n");


	// DO
	//TODO: dummy implementation for dev
	settings[current_setting].acceleration_offset.x += 100;
	settings[current_setting].acceleration_offset.y += 100;
	settings[current_setting].acceleration_offset.z += 100;


	printf("Current Position is now new zero!");
	_delay_ms(2000.0);

	next_state = STATE_MAIN_MENU;
	// EXIT

}


void configuration_terminal_state_accelerationsensor_set_scalingfactor(void)
{
	// ENTRY
	configuration_terminal_clear_all();

	printf("=== ACCELERATIONSENSOR SCALING FACTOR ===\n\n");
	printf("Current value: %u\n\n", settings[current_setting].position_multiplier);

	// DO
	settings[current_setting].position_multiplier = configuration_terminal_get_integer(
			settings[current_setting].position_multiplier, 1, UINT16_MAX);

	next_state = STATE_MAIN_MENU;

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

