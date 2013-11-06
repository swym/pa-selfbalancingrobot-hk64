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
#include "lib/uart.h"
#include "lib/twi_master.h"

#include "acceleration_t.h"
#include "bma020.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void main_init(void);
static void main_run(void);

/* *** FUNCTION DEFINITIONS ************************************************** */

void main_init(void)
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definierten */

	UART_init(9600);	/* Init UART mit 9600 baud */

	twi_master_init();		/* Init TWI/I2C Schnittstelle */

	sei();				/* Enable global interrupts */
}


void main_run(void)
{
	uint8_t led = 0;

	for(;;) {
		PORTC = led;
		led++;

		_delay_ms(500);
	}
}


/**
 * @details Does something important
 * @return
 */
int main(void)
{
	main_init();
	main_run();

	return 0;
}



