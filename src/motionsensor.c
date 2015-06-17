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

#include "filter.h"
/* * local headers               * */
#include "mpu9150.h"
#include "leds.h"



/* *** DEFINES ************************************************************** */
//#define RAD2DEG100 5729.57
//#define NORMALIZATION_ACCELERATION_ANGLE	2000
//#define NORMALIZATION_ACCELERATION_ANGLE	7321
//#define NORMALIZATION_ACCELERATION_ANGLE  5215  //convert rad from atan (+-pi/2) to 2^13 => (2^13 / (pi/2)) = 5215.189
//#define NORMALIZATION_ACCELERATION_ANGLE    1640


#define ANGULARVELOCITY_DELTA_TIME_MS		250			// gyro * dt =  angular * 4 / 1000
//integrate angular velocity to angle over time (dt = 4 ms)
// gyro * dt * correction;
// dt = 4 * 1000;
// gryo * dt = gyro * 4 / 1000 = gyro / 250
// correction value = 1/979.86 (empirisch gesucht) sollte 1000 sein ?!?!?
// gyro / 250 * 1/979.86 = gyro / 244965.240
//#define NORMALIZATION_ANGULARVELOCITY_ANGLE	244965.240  //  * ,123478261 (2^13 / 2300)
#define NORMALIZATION_ANGULARVELOCITY_ANGLE 250000.0



/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

//there a three object of each sensor value: offset_vector; vector with concret data; filter buffer
//also a rawdata buffer
static motionsensor_motiondata_t motiondata;
static motionsensor_motiondata_t offset;
static mpu9150_data_t raw_imudata;

//static acceleration_vector_t acceleration_vector;
//static angularvelocity_vector_t angularvelocity_vector;
//static int16_t	temperature;

//static acceleration_vector_t acceleration_offset_vector;
//static angularvelocity_vector_t angularvelocity_offset_vector; //TODO: replace with temp. sensitive solution

static float valid_acceleration_magnitude_upper_limit;
static float valid_acceleration_magnitude_lower_limit;

// - FILTER
//static uint8_t angularvelocity_filter_mask[] = {64, 1, 0, 0, 0, 0, 0, 0};
static uint8_t angularvelocity_filter_mask[] = {1, 0, 0, 0, 0, 0, 0, 0};
static filter_moving_generic_average_t angularvelocity_filter_x;
static filter_moving_generic_average_t angularvelocity_filter_y;
static filter_moving_generic_average_t angularvelocity_filter_z;

static uint8_t acceleration_filter_mask[] = {1, 1, 1, 1, 1, 1, 1, 1};
//static uint8_t acceleration_filter_mask[] = {4, 2, 2, 2, 1, 1, 1, 1};
//static uint8_t acceleration_filter_mask[] = {1, 0, 0, 0, 0, 0, 0, 0};
//static uint8_t acceleration_filter_mask[] = {4, 2, 2, 1, 1, 1, 0, 0};
static filter_moving_generic_average_t acceleration_filter_x;
static filter_moving_generic_average_t acceleration_filter_y;
static filter_moving_generic_average_t acceleration_filter_z;

static filter_moving_average_float_t acceleration_angle_average;

static uint8_t calibrate_zero_point_filter_mask[] = {1, 1, 1, 1, 1, 1, 1, 1};


// variables used in get_angle_y -> module internal - so print function can read them
static float angle_y;
static float acceleration_angle_y;
static float acceleration_angle_y_magnitude;

static int16_t angle_scalingfactor;

static float complementary_filter_angularvelocity_factor;
static float complementary_filter_acceleraton_factor;


/* * local function declarations * */
static void motionsensor_read_motiondata(motionsensor_motiondata_t * mdata);

/* *** FUNCTION DEFINITIONS ************************************************* */

float motionsensor_get_angle_y(void)
{
	float accel_x_float;
	float accel_z_float;

	//get sensordata data
	motionsensor_read_motiondata(&motiondata);

	//## get acceleration_angle of y
	//##############################
	//determine angle using acceleration vectors and atan and normalize
	accel_x_float = motiondata.acceleration.x;
	accel_z_float = motiondata.acceleration.z;

	accel_x_float = accel_x_float*accel_x_float;
	accel_z_float = accel_z_float*accel_z_float;
	//accel_y not used and should always near 1.0
	acceleration_angle_y_magnitude = sqrt(accel_x_float + 1.0 + accel_z_float) / MOTIONSENSOR_ACCELERATION_1G_FLOAT;

	//calculate angle with acceleration using atan2 and respect magnitude
	acceleration_angle_y  = atan2(motiondata.acceleration.x, motiondata.acceleration.z) / acceleration_angle_y_magnitude;
	acceleration_angle_y = -acceleration_angle_y;

	//filter acceleration angle
	filter_moving_average_float_init(&acceleration_angle_average, acceleration_angle_y);

	//## get angularvelocity of y
	//###########################
	//integrate (sum) to angle_y
	angle_y += motiondata.angularvelocity.y / NORMALIZATION_ANGULARVELOCITY_ANGLE;

	//## fuse angles
	//##############
	//compensate gyro integral error using acceleration_angle
	//grad der qualität des winkels des beschleunigungssensors bewerten
	//if(acceleration_angle_y_magnitude < 1.25 && acceleration_angle_y_magnitude > 0.75) {
	if(acceleration_angle_y_magnitude < valid_acceleration_magnitude_upper_limit &&
	   acceleration_angle_y_magnitude > valid_acceleration_magnitude_lower_limit) {

		//fuse sensor data with complementary filter
		//PORT_LEDS |= _BV(LED2);
		angle_y = angle_y * complementary_filter_angularvelocity_factor +
				  acceleration_angle_average.avg * complementary_filter_acceleraton_factor;
	} else {
		//PORT_LEDS &= ~_BV(LED2);
	}

	return angle_y * (float)angle_scalingfactor;
}

void motionsensor_read_motiondata(motionsensor_motiondata_t * mdata)
{
	//get raw data from imu
	mpu9150_read_motiondata(&raw_imudata);
	//raw_imudata.acceleration.z = -raw_imudata.acceleration.z;	//invert z-axis to correct sign; yt heck?!

	//apply filter
	filter_moving_generic_average_put_element(&acceleration_filter_x, raw_imudata.acceleration.x);
	filter_moving_generic_average_put_element(&acceleration_filter_y, raw_imudata.acceleration.y);
	filter_moving_generic_average_put_element(&acceleration_filter_z, raw_imudata.acceleration.z);

	filter_moving_generic_average_put_element(&angularvelocity_filter_x, raw_imudata.angularvelocity.x);
	filter_moving_generic_average_put_element(&angularvelocity_filter_y, raw_imudata.angularvelocity.y);
	filter_moving_generic_average_put_element(&angularvelocity_filter_z, raw_imudata.angularvelocity.z);

	mdata->temperature = raw_imudata.temperature;

	//apply offset
	mdata->acceleration.x = acceleration_filter_x.avg + offset.acceleration.x;
	mdata->acceleration.y = acceleration_filter_y.avg + offset.acceleration.y;
	mdata->acceleration.z = acceleration_filter_z.avg + offset.acceleration.z;

	mdata->angularvelocity.x = raw_imudata.angularvelocity.x + offset.angularvelocity.x;
	mdata->angularvelocity.y = raw_imudata.angularvelocity.y + offset.angularvelocity.y;
	mdata->angularvelocity.z = raw_imudata.angularvelocity.z + offset.angularvelocity.z;
}


void motionsensor_calibrate_zero_point(void)
{
	uint8_t i;

	//set offsets to zero
	offset.acceleration.x = 0;
	offset.acceleration.y = 0;
	offset.acceleration.z = 0;

	offset.angularvelocity.x = 0;
	offset.angularvelocity.y = 0;
	offset.angularvelocity.z = 0;

	//reset filters - use
	filter_moving_generic_average_init(&acceleration_filter_x, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_y, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_z, calibrate_zero_point_filter_mask, 0);

	filter_moving_generic_average_init(&angularvelocity_filter_x, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_y, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_z, calibrate_zero_point_filter_mask, 0);

	//loop: read rawdata + filter them...
	for(i = 0;i < FILTER_MOVING_GENERIC_WEIGHTS_COUNT * 8;i++) {
		motionsensor_read_motiondata(&motiondata);
	}

	//take current values and set them as offsets
	offset.acceleration.x = -motiondata.acceleration.x;
	offset.acceleration.y = -motiondata.acceleration.y;
	offset.acceleration.z = (MOTIONSENSOR_ACCELERATION_1G - motiondata.acceleration.z);

	//TODO: replace with temp sensitiv solution
	offset.angularvelocity.x = -motiondata.angularvelocity.x;
	offset.angularvelocity.y = -motiondata.angularvelocity.y;
	offset.angularvelocity.z = -motiondata.angularvelocity.z;

	//restore filter profiles
	filter_moving_generic_average_init(&acceleration_filter_x, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_y, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_z, acceleration_filter_mask, MOTIONSENSOR_ACCELERATION_1G);

	filter_moving_generic_average_init(&angularvelocity_filter_x, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_y, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_z, angularvelocity_filter_mask, 0);

	//read dummy data to fill up filters again
	for(i = 0;i < FILTER_MOVING_GENERIC_WEIGHTS_COUNT * 8;i++) {
		motionsensor_read_motiondata(&motiondata);
	}
}

/**
 * returns as call by reference the current offset.
 * @param acceleration
 */
void motionsensor_get_acceleration_offset_vector(acceleration_vector_t *accel_v)
{
	accel_v->x = offset.acceleration.x;
	accel_v->y = offset.acceleration.y;
	accel_v->z = offset.acceleration.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param acceleration
 */
void motionsensor_set_acceleration_offset_vector(acceleration_vector_t *accel_v)
{
	if(accel_v != NULL) {
		offset.acceleration.x = accel_v->x;
		offset.acceleration.y = accel_v->y;
		offset.acceleration.z = accel_v->z;
	} else {
		offset.acceleration.x = 0;
		offset.acceleration.y = 0;
		offset.acceleration.z = 0;
	}
}

/**
 * returns as call by reference the current offset.
 * @param angularvelocity
 */
void motionsensor_get_angularvelocity_offset_vector(angularvelocity_vector_t *angular_v)
{
	angular_v->x = offset.angularvelocity.x;
	angular_v->y = offset.angularvelocity.y;
	angular_v->z = offset.angularvelocity.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param angularvelocity
 */
void motionsensor_set_angularvelocity_offset_vector(angularvelocity_vector_t *angular_v)
{
	if(angular_v != NULL) {
		offset.angularvelocity.x = angular_v->x;
		offset.angularvelocity.y = angular_v->y;
		offset.angularvelocity.z = angular_v->z;
	} else {
		offset.angularvelocity.x = 0;
		offset.angularvelocity.y = 0;
		offset.angularvelocity.z = 0;
	}
}

uint16_t motionsensor_get_angle_scalingfactor(void)
{
	return angle_scalingfactor;
}

void motionsensor_set_angle_scalingfactor(uint16_t s)
{
	angle_scalingfactor = s;
}


float motionsensor_get_complementary_filter_ratio(void)
{
	//angularvelocity_factor == b; acceleration_factor == 1-b
	return complementary_filter_angularvelocity_factor;
}


//angularvelocity_factor == b; acceleration_factor == 1-b
void motionsensor_set_complementary_filter_ratio(float ratio)
{
	if(ratio > MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE) {
		ratio = MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE;
	}

	complementary_filter_angularvelocity_factor = ratio;
	complementary_filter_acceleraton_factor     = MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE - ratio;
}

float motionsensor_get_valid_acceleration_magnitude(void)
{
	//return only threshold
	return valid_acceleration_magnitude_upper_limit - 1.0;
}
void motionsensor_set_valid_acceleration_magnitude(float valid_magnitude)
{
	//normal == 1 G == 1.0
	//set upper and lower limit
	valid_acceleration_magnitude_upper_limit = 1.0 + valid_magnitude;
	valid_acceleration_magnitude_lower_limit = 1.0 - valid_magnitude;
}

void motionsensor_get_raw_motiondata(motionsensor_motiondata_t * mdata)
{

	mdata->acceleration.x = raw_imudata.acceleration.x;
	mdata->acceleration.y = raw_imudata.acceleration.y;
	mdata->acceleration.z = raw_imudata.acceleration.z;

	mdata->angularvelocity.x = raw_imudata.angularvelocity.x;
	mdata->angularvelocity.y = raw_imudata.angularvelocity.y;
	mdata->angularvelocity.z = raw_imudata.angularvelocity.z;

	mdata->temperature = raw_imudata.temperature;
}

void motionsensor_get_filtered_motiondata(motionsensor_motiondata_t * mdata)
{

	mdata->acceleration.x = motiondata.acceleration.x;
	mdata->acceleration.y = motiondata.acceleration.y;
	mdata->acceleration.z = motiondata.acceleration.z;

	mdata->angularvelocity.x = motiondata.angularvelocity.x;
	mdata->angularvelocity.y = motiondata.angularvelocity.y;
	mdata->angularvelocity.z = motiondata.angularvelocity.z;

	mdata->temperature = motiondata.temperature;
}

float motionsensor_get_angle_acceleration(void)
{
	return acceleration_angle_y;
}

float motionsensor_get_angle_acceleration_magnitude(void)
{
	return acceleration_angle_y_magnitude;
}

void motionsensor_init(void)
{
	//init under laying hardware
	mpu9150_init();

	//init offset structures
	offset.acceleration.x = 0;
	offset.acceleration.y = 0;
	offset.acceleration.z = 0;

	offset.angularvelocity.x = 0;
	offset.angularvelocity.y = 0;
	offset.angularvelocity.z = 0;

	//init filters
	filter_moving_generic_average_init(&acceleration_filter_x, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_y, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_z, acceleration_filter_mask, MOTIONSENSOR_ACCELERATION_1G);

	filter_moving_generic_average_init(&angularvelocity_filter_x, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_y, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_z, angularvelocity_filter_mask, 0);

	filter_moving_average_float_init(&acceleration_angle_average, 0.0);

	angle_scalingfactor = 1;

	complementary_filter_angularvelocity_factor = MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE;
	complementary_filter_acceleraton_factor = 0;

	valid_acceleration_magnitude_upper_limit = 1.50;
	valid_acceleration_magnitude_lower_limit = 0.50;

}
