/*
 * motionsensor.c
 *
 *  Created on: Jun 28, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "motionsensor.h"

/* * system headers              * */
#include <math.h>

/* * local headers               * */
//#include "bma020.h"
#include "mpu9150.h"
#include "moving_average.h"



/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

static moving_average_t average_rotation_x;
static moving_average_t average_rotation_y;
static moving_average_t average_rotation_z;

static moving_average_t average_acceleration_x;
static moving_average_t average_acceleration_y;
static moving_average_t average_acceleration_z;


/* * local function declarations * */
//static void motionsensor_get_current_rotation(rotation_t *rotation);
//static void motionsensor_get_current_acceleration(acceleration_t acceleration);

/* *** FUNCTION DEFINITIONS ************************************************* */
double motionsensor_get_position()
{
	double RAD2DEG = 57.295;

	double position;

	rotation_t tmp_rotation;
	acceleration_t tmp_acceleration;

	motionsensor_get_current_rotation(&tmp_rotation);
	motionsensor_get_current_acceleration(&tmp_acceleration);

	position = atan2(tmp_acceleration.x, tmp_acceleration.z);

	return (position * RAD2DEG);
}

void motionsensor_get_current_rotation(rotation_t *rotation)
{
	rotation_t new_rotation;

	mpu9150_read_rotation(&new_rotation);

	//do offset calculation?

	moving_average_simple_put_element(&average_rotation_x, new_rotation.x);
	moving_average_simple_put_element(&average_rotation_y, new_rotation.y);
	moving_average_simple_put_element(&average_rotation_z, new_rotation.z);

	//prepare acceleration struct
	rotation->x = average_rotation_x.mean;
	rotation->y = average_rotation_y.mean;
	rotation->z = average_rotation_z.mean;

}
void motionsensor_get_current_acceleration(acceleration_t *acceleration)
{
	acceleration_t new_acceleration;

	mpu9150_read_acceleration(&new_acceleration);

	//TODO: replace with real offset

	moving_average_simple_put_element(&average_acceleration_x, new_acceleration.x);
	moving_average_simple_put_element(&average_acceleration_y, new_acceleration.y);
	moving_average_simple_put_element(&average_acceleration_z, new_acceleration.z);

	//prepare acceleration struct
	acceleration->x = average_acceleration_x.mean;
	acceleration->y = average_acceleration_y.mean;
	acceleration->z = average_acceleration_z.mean;

}


void motionsensor_init()
{
	uint8_t i;

	//init under laying hardware
	//bma020_init();

	mpu9150_init();

	//fill buffer of the average with values
	for(i = 0;i < (4 * MOVING_AVERAGE_ELEMENT_COUNT);i++) {
		motionsensor_get_position();
	}
}
