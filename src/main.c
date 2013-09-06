/*
 * main.c
 *
 *  Created on: 04.09.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "main.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void main_init(void);
static void main_run(void);


/* *** FUNCTION DEFINITIONS ************************************************** */
static void main_init(void)
{
	DDRC = 0xFF;
}


static void main_run(void)
{
	uint8_t led = 1;

	for(;;) {

		PORTC = led;

		led++;
		_delay_ms(1000);
	}
}

int main(void)
{
	main_init();
	main_run();
}


