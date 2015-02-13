/*
 * test_motor-driver.c
 *
 *  Created on: Feb 12, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */

/* * system headers              * */
#include <util/delay.h>

/* * local headers               * */
#include "../l6205pd.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */
void test_motor_driver_init(void);


/* *** FUNCTION DEFINITIONS ************************************************* */


void test_motor_driver(void)
{
	test_motor_driver_init();

	uint8_t i;


	for(i = 0;i < 255;i++) {
		motor_set_speed(MOTOR_1, 128);
		motor_set_speed(MOTOR_2, 128);

		_delay_ms(2000);

		motor_set_speed(MOTOR_1, 0);
		motor_set_speed(MOTOR_2, 0);

		_delay_ms(2000);

		motor_set_speed(MOTOR_1, 255);
		motor_set_speed(MOTOR_2, 255);

		_delay_ms(2000);

	}
}

void test_motor_driver_init(void)
{
	init_motors();

	sei();
}
