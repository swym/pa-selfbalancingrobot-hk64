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
#include <avr/interrupt.h>

#include "../filter.h"
/* * local headers               * */
#include "../uart.h"
#include "../leds.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
//weighted_average_t wavg;
//moving_average_t   mavg;

filter_moving_generic_average_t gen_avg;
filter_moving_average_float_t float_avg;
filter_moving_average_t int_avg;

int16_t test_values[] = {1000, 0, -2333, 23, 544,
						123, 330, 100, 0, -222,
						345, 30, 10000, 10000, 12000,
						15000, 10000, 0, -10000, -12000};

float test_floats[] = {1000.0, 0.0, -2333.0, 23.0, 544.0,
					   123.0, 330.0, 100.0, 0.0, -222.0,
					   345.0, 30.0, 10000.0, 10000.0, 12000.0,
				       15000.0, 10000.0, 0.0, -10000.0, -12000.0};

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
	uart_init(UART_BAUDRATE_115k);	/* Init UART mit 115k baud */
	uart_init_stdio();
	sei();
}

void test_filters_run()
{
	uint8_t i;
	uint8_t j;
	test_filters_init();



	i = 0;
		for(;;) {

		printf("test generic_8_mean...\n");
		_delay_ms(1000);

		filter_moving_generic_average_init(&gen_avg, weights_8_mean, 0);

		//printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0x0F;
			filter_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}
		printf("\n\n\n");

		// ####################################################################################

		printf("test new int mean...\n");
		_delay_ms(1000);

		filter_moving_average_create(&int_avg, 16, 0);

		//printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);
		printf("ecnt: %d\n", int_avg.elements_count);
		printf("sum: %ld\n", int_avg.sum);
		printf("ecnt: %d\n", int_avg.avg);

		for(i = 0;i < int_avg.elements_count;i++) {
			printf("%d ", int_avg.elements[i]);
		}
		printf("\n");

		printf("insert %d values...\n", test_values_counter);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0x0F;
			filter_moving_average_insert(&int_avg, test_values[i]);
			PORT_LEDS = 0x00;

			printf("n:%7d -> ", test_values[i]);

			for(j = 0; j < int_avg.elements_count;j++) {
				printf("%d:%7d ", j, int_avg.elements[j]);
			}

			printf(" sum%7ld ", int_avg.sum);
			printf(" avg%7d ", int_avg.avg);

			printf("\n");



			_delay_ms(5);
		}

		printf("insert %d values...\n", test_values_counter);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0x0F;
			filter_moving_average_insert(&int_avg, test_values[i]);
			PORT_LEDS = 0x00;

			printf("n:%7d -> ", test_values[i]);

			for(j = 0; j < int_avg.elements_count;j++) {
				printf("%d:%7d ", j, int_avg.elements[j]);
			}

			printf(" sum%7ld ", int_avg.sum);
			printf(" avg%7d ", int_avg.avg);

			printf("\n");


			_delay_ms(5);
		}

		printf("flush...\n");

		filter_moving_average_flush(&int_avg);

		for(i = 0; i < test_values_counter; i++) {

			PORT_LEDS = 0x0F;
			filter_moving_average_insert(&int_avg, test_values[i]);
			PORT_LEDS = 0x00;

			printf("n:%7d -> ", test_values[i]);

			for(j = 0; j < int_avg.elements_count;j++) {
				printf("%d:%7d ", j, int_avg.elements[j]);
			}

			printf(" sum%7ld ", int_avg.sum);
			printf(" avg%7d ", int_avg.avg);

			printf("\n");


			_delay_ms(5);
		}
		printf("\n\n\n");


		// ####################################################################################

		printf("test new float mean...\n");
		_delay_ms(1000);

		filter_moving_average_float_create(&float_avg, 16, 0.0F);

		//printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);
		printf("ecnt: %d\n", float_avg.elements_count);
		printf("sum: %f\n", float_avg.sum);
		printf("ecnt: %f\n", float_avg.avg);

		for(i = 0;i < float_avg.elements_count;i++) {
			printf("%f ", float_avg.elements[i]);
		}
		printf("\n");

		printf("insert %d values...\n", test_values_counter);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xF0;
			filter_moving_average_float_insert(&float_avg, test_floats[i]);
			PORT_LEDS = 0x00;

			printf("n:%7f -> ", test_floats[i]);

			for(j = 0; j < float_avg.elements_count;j++) {
				printf("%d:%7f ", j, float_avg.elements[j]);
			}

			printf(" sum%7f ", float_avg.sum);
			printf(" avg%7f ", float_avg.avg);

			printf("\n");



			_delay_ms(5);
		}

		printf("\n\n\n");





		// ####################################################################################
/*
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
/*
		printf("test weights_fat...\n");
		_delay_ms(1000);

		filter_moving_generic_average_init(&gen_avg, weights_fat, 0);
		printf("wcnt:%d wsum:%d\n",gen_avg.weights_count, gen_avg.weights_sum);

		for(i = 0; i < test_values_counter; i++) {
			PORT_LEDS = 0xFF;
			filter_moving_generic_average_put_element(&gen_avg, test_values[i]);
			PORT_LEDS = 0x00;
			printf("%7d", gen_avg.avg);
			_delay_ms(5);
		}
		printf("\n\n\n");
*/
		// ####################################################################################

//		printf("test float_avg...\n");
//		_delay_ms(1000);
//
//		filter_moving_average_float_init(&float_avg, 0.0);
//
//		for(i = 0; i < test_values_counter; i++) {
//			PORT_LEDS = 0xF0;
//			filter_moving_average_float_put_element(&float_avg, test_values[i]);
//			PORT_LEDS = 0x00;
//			printf("%7f", float_avg.avg);
//			_delay_ms(5);
//		}
//		printf("\n\n\n");

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
