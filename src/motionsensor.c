/*
 * motionsensor.c
 *
 *  Created on: Jun 28, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "motionsensor.h"

/* * system headers              * */
#include <stdlib.h>
#include <math.h>

#include <stdio.h>

/* * local headers               * */
//#include "bma020.h"
#include "mpu9150.h"
#include "moving_average.h"



/* *** DEFINES ************************************************************** */
#define RAD2DEG 5729.57

#define COMPFILTER_GRYO_FACTOR  0.95
#define COMPFILTER_ACCEL_FACTOR 0.05

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

static acceleration_t acceleration_offset;
static angularvelocity_t angularvelocity_offset;

static double position_multiplier;

static moving_average_t average_angularvelocity_x;
static moving_average_t average_angularvelocity_y;
static moving_average_t average_angularvelocity_z;

static moving_average_t average_acceleration_x;
static moving_average_t average_acceleration_y;
static moving_average_t average_acceleration_z;

static int16_t gyro_angle_y;


/* * local function declarations * */
//static void motionsensor_get_current_rotation(rotation_t *rotation);
//static void motionsensor_get_current_acceleration(acceleration_t acceleration);

/* *** FUNCTION DEFINITIONS ************************************************* */
int16_t motionsensor_get_position()
{
	acceleration_t cur_acceleration;  //current acceleration
	int16_t cur_angularvelocity_y;    //current angularvelocity
	int16_t accel_angle_y;			  //calculated angle from acceleration sensor
	int16_t angle_y;			      //fusioned angle


	cur_angularvelocity_y = mpu9150_read_angularvelocity_y();
	motionsensor_get_current_acceleration(&cur_acceleration);

	//integrate angular velocity to angle over time (dt = 4 ms)
	gyro_angle_y += (cur_angularvelocity_y * 4/1000);

	//determine angle using acceleration vectors and atan
	accel_angle_y = (int16_t)((atan2(cur_acceleration.x, cur_acceleration.z) * RAD2DEG));

	//sensordata fusion with a complementary filter
	//angle_y = (0.3 * gyro_angle_y) + (0.7 * accel_angle_y); //ok, aber sehr verrauscht
	angle_y = (COMPFILTER_GRYO_FACTOR * gyro_angle_y) + (COMPFILTER_ACCEL_FACTOR * accel_angle_y);



//	printf("% 5d% 5d% 5d\n",gyro_angle_y, accel_angle_y, angle_y);

	return angle_y;

}

void motionsensor_get_current_angularvelocity(angularvelocity_t *angularvelocity)
{
	angularvelocity_t new_angularvelocity;

	mpu9150_read_angularvelocity(&new_angularvelocity);

	//correct with offset
	new_angularvelocity.x += angularvelocity_offset.x;
	new_angularvelocity.y += angularvelocity_offset.y;
	new_angularvelocity.z += angularvelocity_offset.z;

	//determine mean
	moving_average_simple_put_element(&average_angularvelocity_x, new_angularvelocity.x);
	moving_average_simple_put_element(&average_angularvelocity_y, new_angularvelocity.y);
	moving_average_simple_put_element(&average_angularvelocity_z, new_angularvelocity.z);

	//prepare acceleration struct
	angularvelocity->x = average_angularvelocity_x.mean;
	angularvelocity->y = average_angularvelocity_y.mean;
	angularvelocity->z = average_angularvelocity_z.mean;
}
void motionsensor_get_current_acceleration(acceleration_t *acceleration)
{
	acceleration_t new_acceleration;

	mpu9150_read_acceleration(&new_acceleration);

	//correct with offset
	new_acceleration.x += acceleration_offset.x;
	new_acceleration.y += acceleration_offset.y;
	new_acceleration.z += acceleration_offset.z;

	//determine mean
	moving_average_simple_put_element(&average_acceleration_x, new_acceleration.x);
	moving_average_simple_put_element(&average_acceleration_y, new_acceleration.y);
	moving_average_simple_put_element(&average_acceleration_z, new_acceleration.z);

	//prepare acceleration struct
	acceleration->x = average_acceleration_x.mean;
	acceleration->y = average_acceleration_y.mean;
	acceleration->z = average_acceleration_z.mean;

}

/**
 * returns as call by reference the current offset.
 * @param accel
 */
void motionsensor_get_acceleration_offset(acceleration_t *acceleration)
{
	acceleration->x = acceleration_offset.x;
	acceleration->y = acceleration_offset.y;
	acceleration->z = acceleration_offset.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param accel
 */
void motionsensor_set_acceleration_offset(acceleration_t *acceleration)
{
	if(acceleration != NULL) {
		acceleration_offset.x = acceleration->x;
		acceleration_offset.y = acceleration->y;
		acceleration_offset.z = acceleration->z;
	} else {
		acceleration_offset.x = 0;
		acceleration_offset.y = 0;
		acceleration_offset.z = 0;
	}
}

/**
 * Return the current posiition multiplier
 * @return current position multiplier
 */
uint16_t motionsensor_get_position_multiplier(void)
{
	return position_multiplier;
}


/**
 * Sets the position multiplier. Range is 1 to 2^16.
 * @param position_multiplier
 */
void motionsensor_set_position_multiplier(uint16_t multiplier)
{
	if(multiplier > 1) {
		position_multiplier = multiplier;
	} else {
		position_multiplier = 1;
	}
}


void motionsensor_set_zero_point(void)
{
	acceleration_t tmp_acceleration;
	angularvelocity_t tmp_angularvelocity;

	int8_t i;
	for(i = 0;i < (4 * MOVING_AVERAGE_ELEMENT_COUNT);i++) {

		motionsensor_get_current_acceleration(&tmp_acceleration);
		motionsensor_get_current_angularvelocity(&tmp_angularvelocity);

		acceleration_offset.x = -tmp_acceleration.x;
		acceleration_offset.y = -tmp_acceleration.y;
		acceleration_offset.z = (INT16_MAX / 2) -tmp_acceleration.z;

		angularvelocity_offset.x = -tmp_angularvelocity.x;
		angularvelocity_offset.y = -tmp_angularvelocity.y;
		angularvelocity_offset.z = -tmp_angularvelocity.z;

	}
}

void motionsensor_init()
{
	acceleration_t tmp_acceleration;
	angularvelocity_t tmp_angularvelocity;

	//init under laying hardware
	mpu9150_init();

	//init offset structures
	acceleration_offset.x = 0;
	acceleration_offset.y = 0;
	acceleration_offset.z = 0;

	angularvelocity_offset.x = 0;
	angularvelocity_offset.y = 0;
	angularvelocity_offset.z = 0;

	position_multiplier = 1.0;

	//fill buffer of the average with values
	uint8_t i;
	for(i = 0;i < (4 * MOVING_AVERAGE_ELEMENT_COUNT);i++) {
		motionsensor_get_current_acceleration(&tmp_acceleration);
		motionsensor_get_current_angularvelocity(&tmp_angularvelocity);
	}
}
