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
#include "filters.h"



/* *** DEFINES ************************************************************** */
#define RAD2DEG100 5729.57
#define NORMALIZATION_RAD2INT14   10430
#define NORMALIZATION_AVELO2INT14 0.0057  // == dt * normalization_factor == (4 / 1000) * 1.425
/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
typedef struct{
	moving_average_t x;
	moving_average_t y;
	moving_average_t z;
} average_vector_t;

static acceleration_t acceleration_offset;
static angularvelocity_t angularvelocity_offset;

static double position_multiplier;

static average_vector_t average_angularvelocity;		//average vector for using with motionsensor_get_current_angularvelocity(angularvelocity_t *angularvelocity);
static moving_average_t average_angularvelocity_y;		//single average for using with motionsensor_get_current_angularvelocity_y();

average_vector_t average_acceleration;

static int16_t integrated_angularvelocity_angle_y;

static double complementary_filter_angularvelocity_factor;
static double complementary_filter_acceleraton_factor;


/* * local function declarations * */
//static void motionsensor_get_current_rotation(rotation_t *rotation);
//static void motionsensor_get_current_acceleration(acceleration_t acceleration);

static int16_t motionsensor_get_current_angularvelocity_y(void);
static inline void reset_integrated_gyro_angle_y();

/* *** FUNCTION DEFINITIONS ************************************************* */
int16_t motionsensor_get_position()
{
	acceleration_t cur_acceleration;  //current acceleration
	int16_t cur_angularvelocity_y;    //current angularvelocity
	int16_t accel_angle_y;			  //calculated angle from acceleration sensor
	int16_t angle_y;			      //fusioned angle


	motionsensor_get_current_acceleration(&cur_acceleration);
	cur_angularvelocity_y = motionsensor_get_current_angularvelocity_y();
	//cur_angularvelocity_y = mpu9150_read_angularvelocity_y();

	//determine angle using acceleration vectors and atan and normalize
	accel_angle_y = (int16_t)((atan2(cur_acceleration.x, cur_acceleration.z) * NORMALIZATION_RAD2INT14));

	//TODO: HACK: to reduce gyro errors, reset integrated gyro if acceleration of x is 0;
	//Use a more stable solution
	if(cur_acceleration.x == 0) {
		reset_integrated_gyro_angle_y();
	}

	//integrate angular velocity to angle over time (dt = 4 ms) and normalize
	//integrated_gyro_angle_y += (cur_angularvelocity_y * (4/1000));
	integrated_angularvelocity_angle_y += cur_angularvelocity_y * NORMALIZATION_AVELO2INT14;

	//sensordata fusion with a complementary filter
	//angle_y = (0.3 * gyro_angle_y) + (0.7 * accel_angle_y); //ok, aber sehr verrauscht
	angle_y = (complementary_filter_angularvelocity_factor * integrated_angularvelocity_angle_y) +
			  (complementary_filter_acceleraton_factor * accel_angle_y);

	return angle_y;


}

static inline void reset_integrated_gyro_angle_y()
{
	integrated_angularvelocity_angle_y = 0;
}

int16_t motionsensor_get_current_angularvelocity_y(void)
{
	//read new value
	int16_t new_angularvelocity_y = mpu9150_read_angularvelocity_y() +
			angularvelocity_offset.y;

	//determine mean
	filters_moving_average_put_element(&average_angularvelocity_y, new_angularvelocity_y);

	//return mean
	return average_angularvelocity_y.mean;
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
	filters_moving_average_put_element(&average_angularvelocity.x, new_angularvelocity.x);
	filters_moving_average_put_element(&average_angularvelocity.y, new_angularvelocity.y);
	filters_moving_average_put_element(&average_angularvelocity.z, new_angularvelocity.z);

	//prepare acceleration struct
	angularvelocity->x = average_angularvelocity.x.mean;
	angularvelocity->y = average_angularvelocity.y.mean;
	angularvelocity->z = average_angularvelocity.z.mean;
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
	filters_moving_average_put_element(&average_acceleration.x, new_acceleration.x);
	filters_moving_average_put_element(&average_acceleration.y, new_acceleration.y);
	filters_moving_average_put_element(&average_acceleration.z, new_acceleration.z);

	//prepare acceleration struct
	acceleration->x = average_acceleration.x.mean;
	acceleration->y = average_acceleration.y.mean;
	acceleration->z = average_acceleration.z.mean;

}

/**
 * returns as call by reference the current offset.
 * @param acceleration
 */
void motionsensor_get_acceleration_offset(acceleration_t *acceleration)
{
	acceleration->x = acceleration_offset.x;
	acceleration->y = acceleration_offset.y;
	acceleration->z = acceleration_offset.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param acceleration
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
 * returns as call by reference the current offset.
 * @param angularvelocity
 */
void motionsensor_get_angularvelocity_offset(angularvelocity_t *angularvelocity)
{
	angularvelocity->x = angularvelocity_offset.x;
	angularvelocity->y = angularvelocity_offset.y;
	angularvelocity->z = angularvelocity_offset.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param angularvelocity
 */
void motionsensor_set_angularvelocity_offset(angularvelocity_t *angularvelocity)
{
	if(angularvelocity != NULL) {
		angularvelocity_offset.x = angularvelocity->x;
		angularvelocity_offset.y = angularvelocity->y;
		angularvelocity_offset.z = angularvelocity->z;
	} else {
		angularvelocity_offset.x = 0;
		angularvelocity_offset.y = 0;
		angularvelocity_offset.z = 0;
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

		mpu9150_read_acceleration(&tmp_acceleration);
		mpu9150_read_angularvelocity(&tmp_angularvelocity);

		acceleration_offset.x = -tmp_acceleration.x;
		acceleration_offset.y = -tmp_acceleration.y;
		acceleration_offset.z = (INT16_MAX / 2) -tmp_acceleration.z;

		angularvelocity_offset.x = -tmp_angularvelocity.x;
		angularvelocity_offset.y = -tmp_angularvelocity.y;
		angularvelocity_offset.z = -tmp_angularvelocity.z;

	}
}

double motionsensor_get_complementary_filter_angularvelocity_factor(void)
{
	return complementary_filter_angularvelocity_factor;
}

void motionsensor_set_complementary_filter_angularvelocity_factor(double factor)
{
	if(factor > 1.0 || factor < 0.0) {
		factor = 1.0 - complementary_filter_acceleraton_factor;
	}

	complementary_filter_angularvelocity_factor = factor;
}

double motionsensor_get_complementary_filter_acceleraton_factor(void)
{
	return complementary_filter_acceleraton_factor;
}

void motionsensor_set_complementary_filter_acceleraton_factor(double factor)
{
	if(factor > 1.0 || factor < 0.0) {
		factor = 1.0 - complementary_filter_angularvelocity_factor;
	}

	complementary_filter_acceleraton_factor = factor;
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

	complementary_filter_angularvelocity_factor = 0.5;
	complementary_filter_acceleraton_factor = 0.5;

	reset_integrated_gyro_angle_y();

	//fill buffer of the average with values
	uint8_t i;
	for(i = 0;i < (4 * MOVING_AVERAGE_ELEMENT_COUNT);i++) {
		motionsensor_get_current_acceleration(&tmp_acceleration);
		motionsensor_get_current_angularvelocity(&tmp_angularvelocity);
		motionsensor_get_current_angularvelocity_y();
	}
}
