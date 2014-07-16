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
weighted_average_t avg;

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_filters_init()
{
	DDR_LED = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */
	DDR_SCOPE = 0xFF;

	UART_init(56700);	/* Init UART mit 38400 baud */
}

void test_filters_run()
{
	test_filters_init();

	printf("hello filters\n");

	filters_weighted_average_put_element(&avg, 1);
	filters_weighted_average_put_element(&avg, 1);
	filters_weighted_average_put_element(&avg, 1);
	filters_weighted_average_put_element(&avg, 1);
	filters_weighted_average_put_element(&avg, 1);

	printf("wavg: %d\n", avg.mean);
}
