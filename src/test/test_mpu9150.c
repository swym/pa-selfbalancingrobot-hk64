/*
 * test_mpu9150.c
 *
 *  Created on: Jun 20, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_mpu9150.h"


/* * system headers              * */
#include <stdlib.h>
#include <stdio.h>

#include <util/delay.h>

/* * local headers               * */
#include "../lib/uart.h"
#include "../lib/twi_master.h"

#include "../mpu9150.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */
static void test_mpu9150_init();

/* *** FUNCTION DEFINITIONS ************************************************* */
void test_mpu9150_run(void)
{
	test_mpu9150_init();
	printf("test_mpu9150_inited\n");

	PORTC |= _BV(1);
	while(true) {

		PORTC ^= _BV(2);

		_delay_ms(5);

		printf("hallo welt! %c\n", mpu9150_get_who_am_i());

	}
}


void test_mpu9150_init()
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */

	UART_init(38400);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */

	PORTC |= _BV(0);

	sei();
}
