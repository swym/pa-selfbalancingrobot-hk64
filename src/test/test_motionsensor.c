/*
 * test_motionsensor.c
 *
 *  Created on: Jun 28, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_motionsensor.h"

/* * system headers              * */
#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

/* * local headers               * */
#include "../lib/uart.h"
#include "../lib/twi_master.h"

#include "../motionsensor.h"



/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */
static void test_motionsensor_init();


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_motionsensor_init()
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */

	UART_init(38400);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */

	PORTC |= _BV(0);

	sei();

	motionsensor_init();
	motionsensor_set_zero_point();
}

void test_motionsensor_run()
{
	test_motionsensor_init();


	printf("test_motionsensor_run\n");


	rotation_t rot_vector;
	acceleration_t accel_vector;

	while(true) {

		PORTC ^= _BV(PC7);

		motionsensor_get_current_acceleration(&accel_vector);
		motionsensor_get_current_rotation(&rot_vector);

		printf("%d:%d:%d:%d:%d:%d:",
				accel_vector.x, accel_vector.y, accel_vector.z,
				rot_vector.x,rot_vector.y, rot_vector.z);

		printf("%d\n",motionsensor_get_position());


		//printf("pos: %f\n", motionsensor_get_position());

//		_delay_ms(1);
	}
}


