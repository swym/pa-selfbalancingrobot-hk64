/*
 * main.c
 *
 *  Created on: 04.09.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
/* modul header                */
#include "main.h"

/* system headers              */
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdbool.h>

/* local headers               */
#include "uart.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void main_init(void);
static void main_run(void);

/* *** FUNCTION DEFINITIONS ************************************************** */
int main(void)
{
	main_init();
	main_run();
}

void main_init(void)
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definierten */

	UART_init(9600);	/* Init UART mit 9600 baud */
}


void main_run(void)
{
	uint8_t led = 1;

	for(;;) {

		PORTC = led;

		printf("LED: %d\n", led);

		led++;
		_delay_ms(1000);
	}
}



