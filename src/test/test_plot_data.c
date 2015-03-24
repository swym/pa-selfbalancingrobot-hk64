/*
 * test_plot_data.c
 *
 *  Created on: Mar 10, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_plot_data.h"

/* * system headers              * */
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

/* * local headers               * */
#include "../timer.h"
#include "../leds.h"
#include "../lib/uart.h"

/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */

/* * local objects               * */

/* * local function declarations * */
static void test_data_plot_init(void);

/* *** FUNCTION DEFINITIONS ************************************************* */
void test_data_plot(void)
{


	test_data_plot_init();
/*
	for(;;) {
		//(b'f', b'\n', -620, 17232, -176, -1348, 0)
		printf("Zgr9lENQ/1D6vAAA\n");

		_delay_ms(500);
	}
*/
	//every timer irq ...

	uint16_t i = 0;

	for(;;) {
		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;
			printf("1234ABCDEF %5u\n",i++);
		}
	}
}

void test_data_plot_init(void)
{
	timer_init();
	UART_init();

	sei();

	printf("inited...\n");
}
