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
#include "../serialport.h"


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
	serialport_init(9600);
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

    	PORT_LEDS = 0x0F;
    	ret = serialport_get_string(output, OUTPUT_SIZE);
    	PORT_LEDS = 0x00;

    	if(ret) {
    		PORT_LEDS = 0xF0;
    		printf("%s\n", output);

    	}

    	printf("disable!\n");
    	serialport_enable_port(false);
    	_delay_ms(2000);
    	serialport_enable_port(true);
    	printf("enable!\n");
    	_delay_ms(2000);
    }
}
