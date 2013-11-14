/*
 * acceleration_t.c
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "acceleration_t.h"

#include "bma020.h"
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


/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */


/**
 * reads a acceleceration vector from the accelerationsensor. also includes the
 * offset.
 *
 * @param acceleration
 */
void acceleration_get_current_acceleration(acceleration_t *acceleration)
{
	//read new value
	bma020_read_raw_acceleration(acceleration);

	//correct with offset
	acceleration->x += offset.x;
	acceleration->y += offset.y;
	acceleration->z += offset.z;
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
}

