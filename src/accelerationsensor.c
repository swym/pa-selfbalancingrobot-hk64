/*
 * acceleration_t.c
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "accelerationsensor.h"

#include <math.h>
#include <stdlib.h>

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
static uint16_t pos_multiplier;


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
void accelerationsensor_get_current_acceleration(acceleration_t *acceleration)
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
 *	Calculates the current position with atan2() and the current acceleration
 *	vector.
 *
 * @return current position
 */
double accelerationsensor_get_current_position()
{
	acceleration_t temp_accel;

	//read current acceleration vectors
	accelerationsensor_get_current_acceleration(&temp_accel);

	//convert to position
	return (atan2(temp_accel.x, temp_accel.z) * pos_multiplier);
}

/**
 * Reads the current acceleration value and sets them as an offset for further
 * messaurements. offset.z is set to 1 G.
 */
void accelerationsensor_calibrate_offset(void)
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
void accelerationsensor_get_offset(acceleration_t *accel)
{
	accel->x = offset.x;
	accel->y = offset.y;
	accel->z = offset.z;
}


/**
 * sets the offset with a given acceleration vector
 * @param accel
 */
void accelerationsensor_set_offset(acceleration_t *accel)
{
	if(accel != NULL) {
		offset.x = accel->x;
		offset.y = accel->y;
		offset.z = accel->z;
	} else {
		offset.x = 0;
		offset.y = 0;
		offset.z = 0;
	}
}

/**
 * Return the current posiition multiplier
 * @return current position multiplier
 */
uint16_t accelerationsensor_get_position_multiplier(void)
{
	return pos_multiplier;
}


/**
 * Sets the position multiplier. Range is 1 to 2^16.
 * @param position_multiplier
 */
void accelerationsensor_set_position_multiplier(uint16_t position_multiplier)
{
	if(position_multiplier > 1) {
		pos_multiplier = position_multiplier;
	} else {
		pos_multiplier = 1;
	}
}

/**
 * Initialize the acceleration module.
 *
 *  - Sets offset to given offset vector or to zero
 *  - Sets position_multiplier to given value or to 1
 *  - reads so long positions till the average mean buffers a filled
 */
void accelerationsensor_init(uint16_t position_multiplier, acceleration_t *accel)
{
	uint8_t i;

	//set position_multiplier
	accelerationsensor_set_position_multiplier(position_multiplier);

	//set offset
	accelerationsensor_set_offset(accel);

	//fill buffer of the average with values
	for(i = 0;i < MOVING_AVERAGE_ELEMENT_COUNT;i++) {
		accelerationsensor_get_current_position();
	}
}

