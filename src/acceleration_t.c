/*
 * acceleration_t.c
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "acceleration_t.h"

#include "bma020.h"
#include "moving_average.h"
/* *** TODO BMA020
 * - Offset automatisch ermitteln
 * - evtl Werte glŠtten/filtern
 *
 *
 *
 * */



/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
static acceleration_t offset;

static moving_average_t average_acceleration_x;
static moving_average_t average_acceleration_y;
static moving_average_t average_acceleration_z;

/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */


/**
 * reads a acceleceration vector from the accelerationsensor. also includes the
 * offset and calculates average
 *
 * @param acceleration
 */
void acceleration_get_current_acceleration(acceleration_t *acceleration)
{
	acceleration_t new_accel;

	//read new value
	bma020_read_raw_acceleration(&new_accel);

	//correct with offset
	new_accel.x += offset.x;
	new_accel.y += offset.y;
	new_accel.z += offset.z;

//	acceleration->x = new_accel.x;
//	acceleration->y = new_accel.y;
//	acceleration->z = new_accel.z;


	//calculate mean
	moving_average_simple_put_element(&average_acceleration_x, new_accel.x);
	moving_average_simple_put_element(&average_acceleration_y, new_accel.y);
	moving_average_simple_put_element(&average_acceleration_z, new_accel.z);

	//prepare acceleration struct
	acceleration->x = average_acceleration_x.mean;
	acceleration->y = average_acceleration_y.mean;
	acceleration->z = average_acceleration_z.mean;
}

/**
 * Reads the current acceleration value and sets them as an offset for further
 * messaurements. offset.z is set to 1 G.
 */
void acceleration_calibrate_offset(void)
{
	acceleration_t temp_accel;

	bma020_read_raw_acceleration(&temp_accel);

	offset.x = -temp_accel.x;
	offset.y = -temp_accel.y;
	offset.z = (INT16_MAX / bma020_get_range()) - temp_accel.z; //offset.z: (INT16_MAX / range) - current.z = 32768 / 2 - current.z
}


/**
 * returns as call by reference the current offset.
 * @param accel
 */
void acceleration_get_offset(acceleration_t *accel)
{
	accel->x = offset.x;
	accel->y = offset.y;
	accel->z = offset.z;
}


/**
 * Initialized the acceleration module.
 *
 * Sets offset to zero.
 */
void acceleration_init(void)
{
	offset.x = 0;
	offset.y = 0;
	offset.z = 0;

	bma020_init();
}

