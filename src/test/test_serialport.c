/*
 * test_serialport.c
 *
 *  Created on: Jun 20, 2015
 *      Author: alexandermertens
 */

#include "test_serialport.h"

/* *** INCLUDES ************************************************************* */

/* * system headers              * */
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <avr/interrupt.h>

/* * local headers               * */
#include "../leds.h"
#include "../uart.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
#define OUTPUT_SIZE 40

/* * local used ext. module objs * */

/* * local objects               * */

/* * local function declarations * */
void test_serialport_init(void);
void test_serialport_run(void);

/* *** FUNCTION DEFINITIONS ************************************************* */

void test_serialport(void)
{
	test_serialport_init();
	test_serialport_run();
}

void test_serialport_init(void)
{
	uart_init(UART_BAUDRATE_9600);
	uart_init_stdio();

	leds_init();

	sei();

	printf("inited...\n");
}

void test_serialport_run(void)
{
	char output[OUTPUT_SIZE];

    bool ret;

    printf("hallo!\n");

    for(;;) {

    	while(uart_available()) {
        	//PORT_LEDS = 0x0F;
        	ret = uart_gets(output, OUTPUT_SIZE);
        	//PORT_LEDS = 0x00;

        	if(ret) {
        		//PORT_LEDS = 0xF0;
        		printf("%s\n", output);
        	}
    	}

    	uart_init(UART_BAUDRATE_9600);
    	printf("disable!\n");
    	uart_enable_rxtx(false);
    	_delay_ms(2000);
    	uart_enable_rxtx(true);
    	uart_init(UART_BAUDRATE_250k);
    	printf("enable!\n");
    	_delay_ms(5000);
    }
}
