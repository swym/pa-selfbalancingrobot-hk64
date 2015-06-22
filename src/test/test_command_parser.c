/*
 * test_command_parser.c
 *
 *  Created on: Jun 17, 2015
 *      Author: alexandermertens
 */
#include "test_command_parser.h"

/* *** INCLUDES ************************************************************* */

/* * system headers              * */
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

/* * local headers               * */
#include "../leds.h"
#include "../uart.h"
#include "../vt100.h"
//#include "../serialport.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */

/* * local objects               * */

/* * local function declarations * */
void test_command_parser_init(void);
void test_command_parser_run(void);

/* *** FUNCTION DEFINITIONS ************************************************* */

void test_command_parser(void)
{
	test_command_parser_init();
	test_command_parser_run();
}

#define STRING_BUFFER_SIZE		40

static FILE uart_str = FDEV_SETUP_STREAM(uart_putc, uart_getc, _FDEV_SETUP_RW);
static FILE stderr_str = FDEV_SETUP_STREAM(uart_putc, NULL, _FDEV_SETUP_WRITE);

static char string_buffer[STRING_BUFFER_SIZE];
static uint8_t string_buffer_index;




bool test_command_gets(char * target, uint8_t target_size)
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

#define OUTPUT_SIZE 50
void test_command_parser_run(void)
{

	char output[OUTPUT_SIZE];


    uart_flush();
    bool ret;

    printf("hallo!\n");

    for(;;) {



    	PORT_LEDS = 0x0F;
    	ret = test_command_gets(output, OUTPUT_SIZE);
    	PORT_LEDS = 0x00;

    	if(ret) {
    		PORT_LEDS = 0xF0;
    		printf("%s\n", output);

    	}



//    	if(uart_available() > MIN_CHARS) {
//
//
//
//
//    		while(uart_available()) {
//
//    			c = (char)uart_getc();
//    			uart_putc('.');
//
//    			//skip if linefeed
//    			if(c != '\r') {
//    				continue;
//    			}
//
//    			if(c != '\n' &&	input_buffer_index < BUFFER_SIZE) {
//    				input_buffer[input_buffer_index++] = c;
//    			} else {
//    				input_buffer[input_buffer_index] = '\0';
//    				uart_flush();
//    				input_buffer_index = 0;
//    			}
//    		}
//
//			uart_flush();
//
//        	printf("\"%s\"", input_buffer);
//
//    	}

    	_delay_ms(250);

    }
//
//    for(;;)
//    {
//        /*
//         * Get received character from ringbuffer
//         * uart_getc() returns in the lower byte the received character and
//         * in the higher byte (bitmask) the last receive error
//         * UART_NO_DATA is returned when no data is available.
//         *
//         */
//        c = uart_getc();
//        if ( c & UART_NO_DATA )
//        {
//            /*
//             * no data available from UART
//             */
//        }
//        else
//        {
//            /*
//             * new data available from UART
//             * check for Frame or Overrun error
//             */
//            if ( c & UART_FRAME_ERROR )
//            {
//                /* Framing Error detected, i.e no stop bit detected */
//                uart_puts_P("UART Frame Error: ");
//            }
//            if ( c & UART_OVERRUN_ERROR )
//            {
//                /*
//                 * Overrun, a character already present in the UART UDR register was
//                 * not read by the interrupt handler before the next character arrived,
//                 * one or more received characters have been dropped
//                 */
//                uart_puts_P("UART Overrun Error: ");
//            }
//            if ( c & UART_BUFFER_OVERFLOW )
//            {
//                /*
//                 * We are not reading the receive buffer fast enough,
//                 * one or more received character have been dropped
//                 */
//                uart_puts_P("Buffer overflow error: ");
//            }
//            /*
//             * send received character back
//             */
//            uart_putc( (unsigned char)c );
//        }
//    }



	/*
	uint8_t chars;
	while(1) {


		if(uart_char_received()) {

			//PORT_LEDS = 0xF0;
			chars = uart_getstring(input_buffer, BUFFER_SIZE);
			//PORT_LEDS = 0x00;



			//fgets(input_buffer, BUFFER_SIZE, stdin);

			//compare = strcmp(input_buffer, "HALT");

			if(input_buffer[0] == 'S') {
				switch (input_buffer[1]) {
					case 'H':
						//PORT_LEDS = 0x00;
						break;
					case 'L':
						//PORT_LEDS = 0x0A;
						break;
					case 'R':
						//PORT_LEDS = 0x0C;
						break;
					case 'F':
						//PORT_LEDS = 0x0D;
						break;
					case 'B':
						//PORT_LEDS = 0x05;
						break;
					default:
						break;
				}
			} else if(input_buffer[0] == 'C' && input_buffer[1] == 'T') {
				//printf("conf\n");
			}

			//printf("%s", input_buffer);

			//_delay_ms(1000);
			uart_clr_rx_buf();
			//PORT_LEDS = 0x00;
		}
	}
	*/
}




void test_command_parser_init(void)
{
	//uart_init(14400);
	leds_init();
    /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro
     *  UART_BAUD_SELECT() (normal speed mode )
     *  or
     *  UART_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
     */
	//serialport_init(9600);
    /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro
     *  UART_BAUD_SELECT() (normal speed mode )
     *  or
     *  UART_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
     */

//	if(USE_DOUBLESPEED) {

//	}
	uart_init(UART_BAUD_SELECT(9600,F_CPU));

    stdout = stdin = &uart_str;
    stderr = &stderr_str;

	sei();

	printf("inited...\n");
}
