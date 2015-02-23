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

/*
typedef struct {
	weighted_average_t x;
	weighted_average_t y;
	weighted_average_t z;
} weighted_average_vector_t;

typedef struct {
	moving_average_t x;
	moving_average_t y;
	moving_average_t z;
} moving_average_vector_t;

typedef struct {
	smooth_t x;
	smooth_t y;
	smooth_t z;
} smooth_vector_t;
*/


/* * local type and constants    * */

/* * local objects               * */

//there a three object of each sensor value: offset_vector; vector with concret data; filter buffer
//also a rawdata buffer

static acceleration_vector_t acceleration_offset_vector;
static angularvelocity_vector_t angularvelocity_offset_vector; //TODO: replace with temp sensitiv solution

static mpu9150_motiondata_t raw_motiondata;

//maybe replace with filter_t_*
static acceleration_vector_t acceleration_vector;
static angularvelocity_vector_t angularvelocity_vector;
static int16_t	temperature;

// persistent value of integrated angularvelocity of y-axis
static int16_t integral_angularvelocity_angle_y;

static int16_t angle_y_scalingfactor;
static double complementary_filter_angularvelocity_factor;
static double complementary_filter_acceleraton_factor;

//* check for deprecation: *//

//static acceleration_t acceleration_offset;
//static angularvelocity_t angularvelocity_offset;

//static smooth_vector_t acceleration_set_zero;
//static smooth_vector_t angularvelocity_set_zero;

//static double position_multiplier;

//static weighted_average_vector_t average_angularvelocity;		//average vector for using with motionsensor_get_current_angularvelocity(angularvelocity_t *angularvelocity);
//static weighted_average_t average_angularvelocity_y;		//single average for using with motionsensor_get_current_angularvelocity_y();

//static weighted_average_vector_t average_acceleration;
//static moving_average_vector_t average_acceleration;
//static smooth_vector_t average_acceleration;


/* * local function declarations * */
//static void motionsensor_get_current_rotation(rotation_t *rotation);
//static void motionsensor_get_current_acceleration(acceleration_t acceleration);

//static int16_t motionsensor_get_current_angularvelocity_y(void);
//static inline void reset_integrated_gyro_angle_y();

static void motionsensor_get_rawdata(void);

static int16_t motionsensor_calc_acceleration_angle_y(void);
static void motionsensor_filter_acceleration_vector(void);

static int16_t motionsensor_calc_angularvelocity_angle_y(void);
static void motionsensor_filter_angularvelocity_vector(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
/*
int16_t motionsensor_get_angle()
{
	acceleration_t cur_acceleration;  //current acceleration
	int16_t cur_angularvelocity_y;    //current angularvelocity
	int16_t accel_angle_y;			  //calculated angle from acceleration sensor
	int16_t angle_y;			      //fusioned angle


	motionsensor_get_current_acceleration(&cur_acceleration);
	cur_angularvelocity_y = motionsensor_get_current_angularvelocity_y();
	//cur_angularvelocity_y = mpu9150_read_angularvelocity_y();

	//TODO: Use value of vector to norm accelerations
	//determine angle using acceleration vectors and atan and normalize
	accel_angle_y = (int16_t)((atan2(cur_acceleration.x, cur_acceleration.z) * NORMALIZATION_RAD2INT14));

	//integrate angular velocity to angle over time (dt = 4 ms) and normalize
	//integrated_gyro_angle_y += (cur_angularvelocity_y * (4/1000));
	integrated_angularvelocity_angle_y += cur_angularvelocity_y * NORMALIZATION_AVELO2INT14;

	//TODO: HACK: to reduce gyro errors, reset integrated gyro if acceleration of x is 0;
	//Use a more stable solution
//	if(cur_acceleration.x < 20 && cur_acceleration.x > -20 ) {
//		reset_integrated_gyro_angle_y();
//	}

	//sensordata fusion with a complementary filter
	//angle_y = (0.3 * gyro_angle_y) + (0.7 * accel_angle_y); //ok, aber sehr verrauscht
	angle_y = (complementary_filter_angularvelocity_factor * integrated_angularvelocity_angle_y) +
			  (complementary_filter_acceleraton_factor * accel_angle_y);

	return angle_y / position_multiplier;


}
*/
/*
static inline void reset_integrated_gyro_angle_y()
{
	integrated_angularvelocity_angle_y = 0;
}
*/
/*
int16_t motionsensor_get_current_angularvelocity_y(void)
{
	//read new value
	int16_t new_angularvelocity_y = (int16_t)mpu9150_read_angularvelocity_y() +
			angularvelocity_offset.y;

	//determine mean
	filters_weighted_average_put_element(&average_angularvelocity_y, new_angularvelocity_y);

	//return mean
	return average_angularvelocity_y.mean;
}
*/


int16_t motionsensor_get_angle_y(void)
{
	int16_t accel_y;
	int16_t angular_y;
	int16_t fused_y;

	//get raw data
	motionsensor_get_rawdata();

	//get acceleration_angle of y
	// //filter (smooth) acceleration_vector
	// //determine angle using acceleration vectors and atan and normalize
	accel_y = motionsensor_calc_acceleration_angle_y();

	//get angularvelocity of y
	// //filter (smooth) angularvelocity_vector
	// //integrate angular velocity to angle over time (dt = 4 ms) and normalize
	angular_y = motionsensor_calc_angularvelocity_angle_y();

	//fuse sensor data with complementary filter
	fused_y = (complementary_filter_acceleraton_factor * accel_y) +
			  (complementary_filter_angularvelocity_factor * angular_y);

	return fused_y / angle_y_scalingfactor;
}

void motionsensor_get_rawdata(void)
{
	//get raw data
	mpu9150_read_motiondata(&raw_motiondata);

	//copy raw data into local buffers to prepare inplace filtering
	acceleration_vector.x = raw_motiondata.acceleration.x;
	acceleration_vector.y = raw_motiondata.acceleration.y;
	acceleration_vector.z = raw_motiondata.acceleration.z;

	angularvelocity_vector.x = raw_motiondata.acceleration.x;
	angularvelocity_vector.y = raw_motiondata.acceleration.y;
	angularvelocity_vector.z = raw_motiondata.acceleration.z;

	temperature = raw_motiondata.temp;
}


int16_t motionsensor_calc_acceleration_angle_y(void)
{
	int16_t acceleration_angle_y;

	motionsensor_filter_acceleration_vector();

	//TODO: Use value of vector to norm accelerations
	//determine angle using acceleration vectors and atan and normalize
	acceleration_angle_y = (int16_t)((atan2(acceleration_vector.x, acceleration_vector.z) * NORMALIZATION_RAD2INT14));

	return acceleration_angle_y;
}

void motionsensor_filter_acceleration_vector(void)
{
	acceleration_vector.x += acceleration_offset_vector.x;
	acceleration_vector.y += acceleration_offset_vector.y;
	acceleration_vector.z += acceleration_offset_vector.z;

	//insert here complex filter method
}


int16_t motionsensor_calc_angularvelocity_angle_y(void)
{
	motionsensor_filter_angularvelocity_vector();

	//integrate angular velocity to angle over time (dt = 4 ms) and normalize
	//integral_angularvelocity_angle_y += (angularvelocity_vector.y * (4/1000) * INT14);
	integral_angularvelocity_angle_y += angularvelocity_vector.y * NORMALIZATION_AVELO2INT14;

	return integral_angularvelocity_angle_y;
}


void motionsensor_filter_angularvelocity_vector(void)
{
	//TODO: replace with temp sensitiv solution
	angularvelocity_vector.x += angularvelocity_offset_vector.x;
	angularvelocity_vector.y += angularvelocity_offset_vector.y;
	angularvelocity_vector.z += angularvelocity_offset_vector.z;

	//insert here complex filter method
}

void motionsensor_calibrate_zero_point(void)
{
	uint8_t i;

	//set offsets to zero
	acceleration_offset_vector.x = 0;
	acceleration_offset_vector.y = 0;
	acceleration_offset_vector.z = 0;

	angularvelocity_offset_vector.x = 0;
	angularvelocity_offset_vector.y = 0;
	angularvelocity_offset_vector.z = 0;


	//reset filters (set zero?)x or use other filter profiles...

	//loop: read rawdata + filter them...
	for(i = 0;i < 255;i++) {
		motionsensor_get_rawdata();
	}

	acceleration_offset_vector.x = -acceleration_vector.x;
	acceleration_offset_vector.y = -acceleration_vector.y;
	acceleration_offset_vector.z = (INT16_MAX / 2) - acceleration_vector.z;  //TOOD: define 1 G somewhere


	//TODO: replace with temp sensitiv solution
	angularvelocity_offset_vector.x = -angularvelocity_vector.x;
	angularvelocity_offset_vector.y = -angularvelocity_vector.y;
	angularvelocity_offset_vector.z = -angularvelocity_vector.z;


	//set new offsets with values in angular- and accel-vector
		//keep in mind of sign/translation (1 G, earth accel)

	//restore filter profiles
	//read dummy data to fill up filters
}

/*
void motionsensor_get_current_angularvelocity(angularvelocity_t *angularvelocity)
{
	angularvelocity_t new_angularvelocity;

	mpu9150_read_angularvelocity(&new_angularvelocity);

	//correct with offset - don't filter
//	angularvelocity->x = raw_angularvelocity.x + angularvelocity_offset.x;
//	angularvelocity->y = raw_angularvelocity.y + angularvelocity_offset.y;
//	angularvelocity->z = raw_angularvelocity.z + angularvelocity_offset.z;

	//correct with offset
	new_angularvelocity.x += angularvelocity_offset.x;
	new_angularvelocity.y += angularvelocity_offset.y;
	new_angularvelocity.z += angularvelocity_offset.z;

	//determine mean
//	filters_weighted_average_put_element(&average_angularvelocity.x, new_angularvelocity.x);
//	filters_weighted_average_put_element(&average_angularvelocity.y, new_angularvelocity.y);
//	filters_weighted_average_put_element(&average_angularvelocity.z, new_angularvelocity.z);

	//prepare acceleration struct
	angularvelocity->x = average_angularvelocity.x.mean;
	angularvelocity->y = average_angularvelocity.y.mean;
	angularvelocity->z = average_angularvelocity.z.mean;


}
*/
/*
void motionsensor_get_current_acceleration(acceleration_t *acceleration)
{
	acceleration_t new_acceleration;

	mpu9150_read_acceleration(&new_acceleration);

	//correct with offset
	new_acceleration.x = new_acceleration.x + acceleration_offset.x;
	new_acceleration.y = new_acceleration.y + acceleration_offset.y;
	new_acceleration.z = new_acceleration.z + acceleration_offset.z;

	//determine mean
	//filters_weighted_average_put_element(&average_acceleration.x, new_acceleration.x);
	//filters_weighted_average_put_element(&average_acceleration.y, new_acceleration.y);
	//filters_weighted_average_put_element(&average_acceleration.z, new_acceleration.z);
	//filters_moving_average_put_element(&average_acceleration.x, new_acceleration.x);
	//filters_moving_average_put_element(&average_acceleration.y, new_acceleration.y);
	//filters_moving_average_put_element(&average_acceleration.z, new_acceleration.z);
	filters_smooth_put_element(&average_acceleration.x, new_acceleration.x);
	filters_smooth_put_element(&average_acceleration.y, new_acceleration.y);
	filters_smooth_put_element(&average_acceleration.z, new_acceleration.z);

	//prepare acceleration struct
	//acceleration->x = average_acceleration.x.mean;
	//acceleration->y = average_acceleration.y.mean;
	//acceleration->z = average_acceleration.z.mean;
	acceleration->x = average_acceleration.x.smoothed;
	acceleration->y = average_acceleration.y.smoothed;
	acceleration->z = average_acceleration.z.smoothed;
}
*/

/**
 * returns as call by reference the current offset.
 * @param acceleration
 */
void motionsensor_get_acceleration_offset_vector(acceleration_vector_t *accel_v)
{
	accel_v->x = acceleration_offset_vector.x;
	accel_v->y = acceleration_offset_vector.y;
	accel_v->z = acceleration_offset_vector.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param acceleration
 */
void motionsensor_set_acceleration_offset_vector(acceleration_vector_t *accel_v)
{
	if(accel_v != NULL) {
		acceleration_offset_vector.x = accel_v->x;
		acceleration_offset_vector.y = accel_v->y;
		acceleration_offset_vector.z = accel_v->z;
	} else {
		acceleration_offset_vector.x = 0;
		acceleration_offset_vector.y = 0;
		acceleration_offset_vector.z = 0;
	}
}

/**
 * returns as call by reference the current offset.
 * @param angularvelocity
 */
void motionsensor_get_angularvelocity_offset_vector(angularvelocity_vector_t *angular_v)
{
	angular_v->x = angularvelocity_offset_vector.x;
	angular_v->y = angularvelocity_offset_vector.y;
	angular_v->z = angularvelocity_offset_vector.z;
}

/**
 * sets the offset with a given acceleration vector
 * @param angularvelocity
 */
void motionsensor_set_angularvelocity_offset_vector(angularvelocity_vector_t *angular_v)
{
	if(angular_v != NULL) {
		angularvelocity_offset_vector.x = angular_v->x;
		angularvelocity_offset_vector.y = angular_v->y;
		angularvelocity_offset_vector.z = angular_v->z;
	} else {
		angularvelocity_offset_vector.x = 0;
		angularvelocity_offset_vector.y = 0;
		angularvelocity_offset_vector.z = 0;
	}
}

uint16_t motionsensor_get_angle_scalingfactor(void)
{
	return angle_y_scalingfactor;
}

void motionsensor_set_angle_scalingfactor(uint16_t s)
{
	angle_y_scalingfactor = s;
}

/*
uint16_t motionsensor_get_position_multiplier(void)
{
	return position_multiplier;
}

void motionsensor_set_position_multiplier(uint16_t multiplier)
{
	if(multiplier > 1) {
		position_multiplier = multiplier;
	} else {
		position_multiplier = 1;
	}
}
*/

/*
void motionsensor_acceleration_calibrate_zero_point(void)
{
	//acceleration_vector_t accel_v;
	mpu9150_acceleration_vector_t raw_temp_accel_v;

	uint8_t i;
	for (i = 0; i < 255; ++i) {
		mpu9150_read_acceleration(&raw_temp_accel_v);

		//TODO: Filter/Smooth/bluring
	}

	acceleration_vector.x = -raw_temp_accel_v.x;
	acceleration_vector.y = -raw_temp_accel_v.y;
	acceleration_vector.z = (INT16_MAX / 2) - raw_temp_accel_v.z; //TOOD: define 1 G somewhere
}
*/

/*
void motionsensor_acceleration_set_zero_point(void)
{
	acceleration_t new_acceleration;

	uint8_t i;
	for(i = 0;i < 255;i++) {

		mpu9150_read_acceleration(&new_acceleration);

		filters_smooth_put_element(&acceleration_set_zero.x, new_acceleration.x);
		filters_smooth_put_element(&acceleration_set_zero.y, new_acceleration.y);
		filters_smooth_put_element(&acceleration_set_zero.z, new_acceleration.z);
	}

	acceleration_offset.x = -acceleration_set_zero.x.smoothed;
	acceleration_offset.y = -acceleration_set_zero.y.smoothed;
	acceleration_offset.z = (INT16_MAX / 2) - acceleration_set_zero.z.smoothed;
}
*/

/*
void motionsensor_angularvelocity_calibrate_zero_point(void)
{
	//angularvelocit_vector_t angular_v;
	mpu9150_angularvelocity_vector_t raw_temp_angular_v;

	uint8_t i;
	for (i = 0; i < 255; ++i) {
		mpu9150_read_angularvelocity(&raw_temp_angular_v);

		//TODO: Filter/Smooth/bluring
	}

	angularvelocity_offset_vector.x = -raw_temp_angular_v.x;
	angularvelocity_offset_vector.y = -raw_temp_angular_v.y;
	angularvelocity_offset_vector.z = -raw_temp_angular_v.z;
}
*/

/*
void motionsensor_angularvelocity_set_zero_point(void)
{
	angularvelocity_t new_angularvelocity;

	uint8_t i;
	for(i = 0;i < 255;i++) {

		mpu9150_read_angularvelocity(&new_angularvelocity);

		filters_smooth_put_element(&angularvelocity_set_zero.x, new_angularvelocity.x);
		filters_smooth_put_element(&angularvelocity_set_zero.y, new_angularvelocity.y);
		filters_smooth_put_element(&angularvelocity_set_zero.z, new_angularvelocity.z);
	}

	angularvelocity_offset.x = -angularvelocity_set_zero.x.smoothed;
	angularvelocity_offset.y = -angularvelocity_set_zero.y.smoothed;
	angularvelocity_offset.z = -angularvelocity_set_zero.z.smoothed;
}
*/

double motionsensor_get_complementary_filter_ratio(void)
{
	//angularvelocity_factor == b; acceleration_factor == 1-b
	return complementary_filter_angularvelocity_factor;
}


//angularvelocity_factor == b; acceleration_factor == 1-b
void motionsensor_set_complementary_filter_ratio(double ratio)
{
	if(ratio > 1.0) {
		ratio = 1.0;
	} else if(ratio < 0.0) {
		ratio = 0.0;
	}

	complementary_filter_angularvelocity_factor = ratio;
	complementary_filter_acceleraton_factor     = 1.0 - ratio;
}

void motionsensor_init(void)
{
	//acceleration_t tmp_acceleration;
	//angularvelocity_t tmp_angularvelocity;

	//init under laying hardware
	mpu9150_init();

	//init offset structures
	acceleration_offset_vector.x = 0;
	acceleration_offset_vector.y = 0;
	acceleration_offset_vector.z = 0;

	angularvelocity_offset_vector.x = 0;
	angularvelocity_offset_vector.y = 0;
	angularvelocity_offset_vector.z = 0;

	angle_y_scalingfactor = 1;

	complementary_filter_angularvelocity_factor = 0.9;
	complementary_filter_acceleraton_factor = 0.1;

	//reset_integrated_gyro_angle_y();

//	average_acceleration.x.factor = 32;
//	average_acceleration.y.factor = 32;
//	average_acceleration.z.factor = 32;

//	acceleration_set_zero.x.factor = 64;
//	acceleration_set_zero.y.factor = 64;
//	acceleration_set_zero.z.factor = 64;

//	angularvelocity_set_zero.x.factor = 64;
//	angularvelocity_set_zero.y.factor = 64;
//	angularvelocity_set_zero.z.factor = 64;

	//fill buffer of the average with values
//	uint8_t i;
//	for(i = 0;i < 24;i++) {
//		motionsensor_get_current_acceleration(&tmp_acceleration);
//		motionsensor_get_current_angularvelocity(&tmp_angularvelocity);
//		motionsensor_get_current_angularvelocity_y();

//		motionsensor_acceleration_set_zero_point();
//		motionsensor_angularvelocity_set_zero_point();

//	}
}
