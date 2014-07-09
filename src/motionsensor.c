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

static moving_average_t average_rotation_x;
static moving_average_t average_rotation_y;
static moving_average_t average_rotation_z;

static moving_average_t average_acceleration_x;
static moving_average_t average_acceleration_y;
static moving_average_t average_acceleration_z;

static int16_t gyro_angle_y;


/* * local function declarations * */
//static void motionsensor_get_current_rotation(rotation_t *rotation);
//static void motionsensor_get_current_acceleration(acceleration_t acceleration);

/* *** FUNCTION DEFINITIONS ************************************************* */
//double motionsensor_get_position()
//{
//	//http://forum.arduino.cc/index.php/topic,58048.0.html
//	double RAD2DEG = 57.295;
//
//	double position;
//
//	rotation_t tmp_rotation;
//	acceleration_t tmp_acceleration;
//
//	motionsensor_get_current_rotation(&tmp_rotation);
////	motionsensor_get_current_acceleration(&tmp_acceleration);
//
//	gyroAngle += (tmp_rotation.y * 20/1000);
//
//	printf("%i, %i\n",tmp_rotation.y, gyroAngle);
//
//	//Integriere gyroRate zu GyroAngle
//
//	return 0.0;
//
//	//position = atan2(tmp_acceleration.x, tmp_acceleration.z);
//
//	//return position_multiplier * (position * RAD2DEG);
//
//}

int16_t motionsensor_get_position()
{
	int16_t angularvelocity_y;  //neue winkelgeschwindigkeit
	int16_t accel_angle_y;
	int16_t angle_y;
	acceleration_t cur_acceleration;

	//motionsensor_get_current_rotation(&cur_rotation);
	angularvelocity_y = mpu9150_read_angularvelocity_y() + 350;
	motionsensor_get_current_acceleration(&cur_acceleration);

	//integrate angular velocity to angle over time (dt = 4 ms)
	gyro_angle_y += (angularvelocity_y * 4/1000);

	//determine angle using acceleration vectors and atan
	accel_angle_y = (int16_t)((atan2(cur_acceleration.x, cur_acceleration.z) * RAD2DEG));

	//sensordata fusion with a complementary filter
	//angle_y = (0.3 * gyro_angle_y) + (0.7 * accel_angle_y); //gut, aber sehr verrauscht
	angle_y = (COMPFILTER_GRYO_FACTOR * gyro_angle_y) + (COMPFILTER_ACCEL_FACTOR * accel_angle_y);



//	printf("% 5d% 5d% 5d\n",gyro_angle_y, accel_angle_y, angle_y);

	return angle_y;

}

void motionsensor_get_current_rotation(angularvelocity_t *angularvelocity)
{
	angularvelocity_t new_angularvelocity;

	mpu9150_read_angularvelocity(&new_angularvelocity);

	//do offset calculation?
	new_angularvelocity.x += 0;
	new_angularvelocity.y += 350;
	new_angularvelocity.z += 0;

	moving_average_simple_put_element(&average_rotation_x, new_angularvelocity.x);
	moving_average_simple_put_element(&average_rotation_y, new_angularvelocity.y);
	moving_average_simple_put_element(&average_rotation_z, new_angularvelocity.z);

	//prepare acceleration struct
	angularvelocity->x = average_rotation_x.mean;
	angularvelocity->y = average_rotation_y.mean;
	angularvelocity->z = average_rotation_z.mean;
}
void motionsensor_get_current_acceleration(acceleration_t *acceleration)
{
	acceleration_t new_acceleration;

	mpu9150_read_acceleration(&new_acceleration);

	//correct with offset
	new_acceleration.x += acceleration_offset.x;
	new_acceleration.y += acceleration_offset.y;
	new_acceleration.z += acceleration_offset.z;

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

	motionsensor_get_current_acceleration(&tmp_acceleration);


	//TODO: regard acceleration range to set correct z-value
	acceleration_offset.x = -tmp_acceleration.x;
	acceleration_offset.y = -tmp_acceleration.y;
	acceleration_offset.z = (INT16_MAX / 2) -tmp_acceleration.z;


	angularvelocity_offset.x = -

}

void motionsensor_init()
{
	//init under laying hardware
	//bma020_init();

	mpu9150_init();

	acceleration_offset.x = 0;
	acceleration_offset.y = 0;
	acceleration_offset.z = 0;

	position_multiplier = 1.0;


	//fill buffer of the average with values
	uint8_t i;
	for(i = 0;i < (4 * MOVING_AVERAGE_ELEMENT_COUNT);i++) {
		motionsensor_get_position();
	}
}
