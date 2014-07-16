/*
 * test_filters.c
 *
 *  Created on: Jul 16, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_filters.h"

/* * system headers              * */
#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>

/* * local headers               * */
#include "../lib/uart.h"
#include "../common.h"

#include "../filters.h"

/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
weighted_average_t wavg;
moving_average_t   mavg;

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_filters_init()
{
	DDR_LED = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */
	DDR_SCOPE = 0xFF;

	UART_init(56700);	/* Init UART mit 38400 baud */

	uint8_t i;

	for(i = 0;i < WEIGHTED_AVERAGE_ELEMENT_COUNT * 10;i++) {
		filters_weighted_average_put_element(&wavg, 0);
		filters_moving_average_put_element(&mavg, 0);
	}

}

void test_filters_run()
{
	test_filters_init();

	printf("test filters\n");


	PORT_SCOPE = 0x01;
	filters_weighted_average_put_element(&wavg, 16660);
	PORT_SCOPE = 0x02;
	filters_moving_average_put_element(&mavg, 16660);
	PORT_SCOPE = 0x00;
	printf("\n");
	filters_weighted_average_put_element(&wavg, 17000);
	filters_moving_average_put_element(&mavg, 17000);
	printf("\n");
	filters_weighted_average_put_element(&wavg, 10000);
	filters_moving_average_put_element(&mavg, 10000);
	printf("\n");
	filters_weighted_average_put_element(&wavg, 13344);
	filters_moving_average_put_element(&mavg, 13344);
	printf("\n");
	PORT_SCOPE = 0x01;
	filters_weighted_average_put_element(&wavg, 16660);
	PORT_SCOPE = 0x02;
	filters_moving_average_put_element(&mavg, 16660);
	PORT_SCOPE = 0x00;
	printf("\n");

	printf("wavg: %d\n", wavg.mean);
	printf("mavg: %d\n", mavg.mean);
}
