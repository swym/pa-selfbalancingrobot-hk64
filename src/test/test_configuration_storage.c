/*
 * test_configuration_storage.c
 *
 *  Created on: Jun 29, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_configuration_storage.h"

/* * system headers              * */
#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <util/delay.h>

/* * local headers               * */
#include "../lib/uart.h"
#include "../lib/twi_master.h"

#include "../configuration_storage.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */
static void test_configuration_storage_init(void);

/* *** FUNCTION DEFINITIONS ************************************************* */
void test_configuration_storage_run(void)
{
	test_configuration_storage_init();

	printf("test_configuration_storage_run\n");

//	configuration_t conf;

//	printf("get existing conf\n");
//	configuration_storage_get_configuration(&conf);
//
//	printf("version: %d\n", conf.version);
//	printf("accel.x: %d\n", conf.motionsensor.acceleration_offset.x);
//	printf("accel.y: %d\n", conf.motionsensor.acceleration_offset.y);
//	printf("accel.z: %d\n", conf.motionsensor.acceleration_offset.z);
//	printf("accel.pm: %d\n", conf.motionsensor.position_multiplier);
//
//	printf("pid.p: %d\n", conf.pid.p_factor);
//	printf("pid.i: %d\n", conf.pid.i_factor);
//	printf("pid.d: %d\n", conf.pid.d_factor);
//	printf("pid.sf: %d\n", conf.pid.scalingfactor);
//
//	conf.motionsensor.acceleration_offset.x = 111;
//	conf.motionsensor.acceleration_offset.y = 222;
//	conf.motionsensor.acceleration_offset.z = 333;
//	conf.motionsensor.position_multiplier = 444;
//
//	conf.pid.p_factor = 555;
//	conf.pid.i_factor = 666;
//	conf.pid.d_factor = 777;
//	conf.pid.scalingfactor = 888;
//
//	configuration_storage_set_configuration(&conf);

	printf("...end\n");

	uint8_t i;
	while(true) {
		i++;
	}

}

void test_configuration_storage_init(void)
{
	UART_init(38400);	/* Init UART mit 38400 baud */

}
