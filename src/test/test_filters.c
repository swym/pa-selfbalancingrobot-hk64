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
#include "../leds.h"

#include "../filters.h"

/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
//weighted_average_t wavg;
//moving_average_t   mavg;

filters_moving_generic_average_t gen_avg;
int16_t test_values[] = {1000, 0, -2333, 23, 544,
						123, 330, 100, 0, -222,
						345, 30, 10000, 10000, 12000,
						15000, 10000, 0, -10000, -12000};

uint8_t test_values_counter = 20;

uint8_t weights_8_mean[] = {1, 1, 1, 1, 1, 1, 1, 1};
uint8_t weights_4_mean[] = {1, 1, 1, 1, 0, 0, 0, 0};
uint8_t weights_smooth[] = {64, 1, 0, 0, 0, 0, 0, 0};
uint8_t weights_curve[]  = {8, 6, 4, 2, 1, 1, 0, 0};
uint8_t weights_fat[]    = {16, 8, 8, 6, 4, 2, 2, 1};

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_filters_init()
{
	leds_init();
	UART_init();	/* Init UART mit 38400 baud */
}

void test_filters_run()
{
	uint8_t i;
	test_filters_init();

	i = 0;
		for(;;) {
	/*
		printf("test weights_8_mean...\n");
		_delay_ms(1000);

		filters_moving_generic_average_init(&gen_avg, weights_8_mean, 0);
		//printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xFF;
			filters_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}
		printf("\n\n\n");


		// ####################################################################################

		printf("test weights_4_mean...\n");
		_delay_ms(1000);

		filters_moving_generic_average_init(&gen_avg, weights_4_mean, 0);
		printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xFF;
			filters_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}
		printf("\n\n\n");

		// ####################################################################################

		printf("test weights_smooth...\n");
		_delay_ms(1000);

		filters_moving_generic_average_init(&gen_avg, weights_smooth, 0);
		printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xFF;
			filters_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}

		printf("\n\n\n");


		// ####################################################################################

		printf("test weights_curve...\n");
		_delay_ms(1000);

		filters_moving_generic_average_init(&gen_avg, weights_curve, 0);
		printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xFF;
			filters_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}
		printf("\n\n\n");

*/

		// ####################################################################################

		printf("test weights_fat...\n");
		_delay_ms(1000);

		filters_moving_generic_average_init(&gen_avg, weights_fat, 0);
		printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xFF;
			filters_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}
		printf("\n\n\n");

		// ####################################################################################



		PORT_LEDS = i++;
		_delay_ms(100);
	}
}

/*
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
*/
