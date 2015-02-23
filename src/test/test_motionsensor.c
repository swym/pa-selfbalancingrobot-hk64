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
#include "../mpu9150.h"

#include "../timer.h"
#include "../vt100.h"

#include "../leds.h"




/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4

//Datatypes iterate higher nibble - lower nibble for sec-number

/* * local objects               * */


/* * local function declarations * */
static void test_motionsensor_init();



/* *** FUNCTION DEFINITIONS ************************************************* */
void test_motionsensor_init()
{
	UART_init();	/* Init UART */
	twi_master_init(TWI_TWBR_VALUE_400);	/* Init TWI/I2C Schnittstelle */
	leds_init();

	//timer_init();

	sei();

	motionsensor_init();

	//motionsensor_set_complementary_filter_acceleraton_factor(0.1);
	//motionsensor_set_complementary_filter_angularvelocity_factor(0.9);
}

void test_motionsensor_run()
{
	test_motionsensor_init();

	printf("test_motionsensor_run\n");

	uint8_t i;
	acceleration_vector_t tmp_accel;
	angularvelocity_vector_t tmp_angular;
	int16_t angle_y;


	printf("print data ...\n");

	for(i = 0;i < 30;i++) {
		motionsensor_printdata();
		_delay_ms(100);
	}

	printf("set zero point\n");
	motionsensor_calibrate_zero_point();

	motionsensor_get_acceleration_offset_vector(&tmp_accel);
	motionsensor_get_angularvelocity_offset_vector(&tmp_angular);

	printf("accel_offset: %d %d %d\n", tmp_accel.x, tmp_accel.y, tmp_accel.z);
	printf("angular_offset: %d %d %d\n", tmp_angular.x, tmp_angular.y, tmp_angular.z);

	_delay_ms(3000);

	printf("print again\n");

	printf("print data ...\n");

	for(i = 0;i < 30;i++) {
		motionsensor_printdata();
		_delay_ms(100);
	}

	printf("set zero point\n");
	motionsensor_calibrate_zero_point();

	motionsensor_get_acceleration_offset_vector(&tmp_accel);
	motionsensor_get_angularvelocity_offset_vector(&tmp_angular);

	printf("accel_offset: %d %d %d\n", tmp_accel.x, tmp_accel.y, tmp_accel.z);
	printf("angular_offset: %d %d %d\n", tmp_angular.x, tmp_angular.y, tmp_angular.z);

	_delay_ms(3000);

	printf("print again\n");

	while(true) {
		motionsensor_printdata();
		_delay_ms(100);
	}
}
