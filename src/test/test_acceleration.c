/*
 * test_acceleration.c
 *
 *  Created on: Nov 12, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "test_acceleration.h"


#include <math.h>
#include <stdio.h>

#include <util/delay.h>

#include "../accelerationsensor.h"
#include "../bma020.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
#define PI 3.141


/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */
void test_acceleration_init_and_calibration(void)
{
	acceleration_t temp_accel;
	uint8_t i;

	accelerationsensor_init(1, NULL);
	bma020_set_range(2);
/*
	for(i = 0;i < 255;i++) {
		acceleration_get_current_acceleration(&temp_accel);
		printf("a: x:%d y:%d z:%d\n", temp_accel.x, temp_accel.y, temp_accel.z);
	}
*/
	accelerationsensor_get_offset(&temp_accel);
	printf("offset: x:%d y:%d z:%d\n", temp_accel.x, temp_accel.y, temp_accel.z);

	accelerationsensor_calibrate_offset();

	accelerationsensor_get_offset(&temp_accel);
	printf("offset: x:%d y:%d z:%d\n", temp_accel.x, temp_accel.y, temp_accel.z);

	_delay_ms(2000.0);

	for(;;) {
		accelerationsensor_get_current_acceleration(&temp_accel);
		printf("a:z:%d\n",temp_accel.z);
	}

}


double test_acceleration_print_accel_and_position(void)
{
	acceleration_t accel;

	accelerationsensor_get_current_acceleration(&accel);

	double x = (double)(accel.x);
	double z = (double)(accel.z);

	double atan_rad = -atan2(x, z);
	double atan_deg = atan_rad * (180/PI);

	return atan_deg;

//	printf("rad:%f deg: %f\n" ,atan_rad, atan_deg);

	//printf("x: %d (%d/%d)\n", local_xi/local_zi, local_xi, local_zi);

	//double pos = atan(quotient);

	//printf("accel: x: %d y: %d z:%d - q: %f pos: %f \n", accel.x, accel.y, accel.z, quotient, z);
}

void test_acceleration_configure_convertion(void)
{
	acceleration_t offset = {-3456, 2048, 1919};

	accelerationsensor_init(1, NULL);
	accelerationsensor_set_offset(&offset);

	double pos, pos_dbl;
	int16_t pos_int;


	for(;;) {

		pos = accelerationsensor_get_current_position();
		pos_dbl = pos * 100000;
		pos_int = (int16_t)(pos_dbl);

		printf("pos: %10f pos_dbl: %10f   pos_int: %10i\n", pos, pos_dbl, pos_int);

		_delay_ms(20.0);
	}
}
