/*
 * serialport.c
 *
 *  Created on: Jun 20, 2015
 *      Author: alexandermertens
 */

#include "serialport.h"

/* *** INCLUDES ************************************************************* */

/* * system headers              * */
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

/* * local headers               * */
#include "uart.h"

/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */

/* * local objects               * */
static char string_buffer[STRING_BUFFER_SIZE];
static uint8_t string_buffer_index;

static FILE uart_str = FDEV_SETUP_STREAM(uart_putc, uart_getc, _FDEV_SETUP_RW);
static FILE stderr_str = FDEV_SETUP_STREAM(uart_putc, NULL, _FDEV_SETUP_WRITE);

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */
bool serialport_get_string(char * target, uint8_t target_size)
{
	char new_char;
	bool str_completed = false;
	uint8_t copy_index = 0;

	while(uart_available()) {

		//read charater from uart fifo
		new_char = (char)uart_getc();

		//read into string buffer until newline or buffer full
		if(new_char != '\n' && string_buffer_index < STRING_BUFFER_SIZE - 1) {

			//skip if linefeed
			if(new_char != '\r') {
				string_buffer[string_buffer_index++] = new_char;
			}

		} else {
			//add string end-mark and reset counter
			string_buffer[string_buffer_index] = '\0';
			string_buffer_index = 0;

			//mark string as completed
			str_completed = true;

			//flush uart fifo
			//implies termination of while loop
			uart_flush();
		}
	}

	//if string completed: copy string to target
	if(str_completed) {
		while(string_buffer[copy_index] != '\0' && copy_index < target_size - 1) {
			target[copy_index] = string_buffer[copy_index];
			copy_index++;
		}
	}

	//add string end-mark to target string‚
	target[copy_index] = '\0';

	return str_completed;
}


void serialport_init(uint32_t baudrate)
{
	serialport_set_baudrate(baudrate);

    stdout = stdin = &uart_str;
    stderr = &stderr_str;

    uart_flush();
}

//enable/disable rx/tx bits
void serialport_enable_port(bool enable)
{
	uart_enable_rxtx(enable);
}


void serialport_set_baudrate(uint32_t baudrate)
{
	//uart_init(UART_BAUD_SELECT(baudrate,F_CPU));
	uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(baudrate, F_CPU));
}
