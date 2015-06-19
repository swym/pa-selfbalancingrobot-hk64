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
#include <avr/pgmspace.h>

/* * local headers               * */
#include "../leds.h"
#include "../uart.h"
#include "../vt100.h"


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

#define BUFFER_SIZE		40
static char input_buffer[BUFFER_SIZE];
static char* buf_ptr;

void test_command_parser_run(void)
{
//	int16_t motor_l;
//	uint8_t leds;
//

    unsigned int c;
    char buffer[7];
    int  num=134;


    /*
     *  Transmit string to UART
     *  The string is buffered by the uart library in a circular buffer
     *  and one character at a time is transmitted to the UART using interrupts.
     *  uart_puts() blocks if it can not write the whole string to the circular
     *  buffer
     */
    uart_puts("String stored in SRAM\n");

    /*
     * Transmit string from program memory to UART
     */
    uart_puts_P("String stored in FLASH\n");

    /*
     * Use standard avr-libc functions to convert numbers into string
     * before transmitting via UART
     */
//    itoa( num, buffer, 10);   // convert interger into string (decimal format)
//    uart_puts(buffer);        // and transmit string to UART

    printf("> ");

    printf("hallo alex! %s\n", input_buffer);

    //buf_ptr = input_buffer;

    uint8_t index;

    uart_flush();

    for(;;) {



    	if(uart_available()) {
    		while(uart_available()) {
    			index = 0;
    			c = (char)uart_getc();

    			if(c != '\r' && c != '\n' && index < BUFFER_SIZE) {
    				input_buffer[index++] = c;
    			}
    			input_buffer[index] = '\0';
    		}


        	printf("%s", input_buffer);
        	uart_flush();
    	}

    	_delay_ms(4);
    	leds_inc();
    }

    for(;;)
    {
        /*
         * Get received character from ringbuffer
         * uart_getc() returns in the lower byte the received character and
         * in the higher byte (bitmask) the last receive error
         * UART_NO_DATA is returned when no data is available.
         *
         */
        c = uart_getc();
        if ( c & UART_NO_DATA )
        {
            /*
             * no data available from UART
             */
        }
        else
        {
            /*
             * new data available from UART
             * check for Frame or Overrun error
             */
            if ( c & UART_FRAME_ERROR )
            {
                /* Framing Error detected, i.e no stop bit detected */
                uart_puts_P("UART Frame Error: ");
            }
            if ( c & UART_OVERRUN_ERROR )
            {
                /*
                 * Overrun, a character already present in the UART UDR register was
                 * not read by the interrupt handler before the next character arrived,
                 * one or more received characters have been dropped
                 */
                uart_puts_P("UART Overrun Error: ");
            }
            if ( c & UART_BUFFER_OVERFLOW )
            {
                /*
                 * We are not reading the receive buffer fast enough,
                 * one or more received character have been dropped
                 */
                uart_puts_P("Buffer overflow error: ");
            }
            /*
             * send received character back
             */
            uart_putc( (unsigned char)c );
        }
    }



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

static FILE uart_str = FDEV_SETUP_STREAM(uart_putc, uart_getc, _FDEV_SETUP_RW);
static FILE stderr_str = FDEV_SETUP_STREAM(uart_putc, NULL, _FDEV_SETUP_WRITE);


void test_command_parser_init(void)
{
	//uart_init(14400);
	//leds_init();
    /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro
     *  UART_BAUD_SELECT() (normal speed mode )
     *  or
     *  UART_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
     */
	uart_init( UART_BAUD_SELECT(9600,F_CPU) );

    stdout = stdin = &uart_str;
    stderr = &stderr_str;


	sei();

	printf("inited...\n");
}
