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
//#define NORMALIZATION_ACCELERATION_ANGLE    5215  //convert rad from atan (+-pi/2) to 2^13 => (2^13 / (pi/2)) = 5215.189
//#define NORMALIZATION_ANGULARVELOCITY_ANGLE	4  //  * ,123478261 (2^13 / 2300)

#define NORMALIZATION_ACCELERATION_ANGLE	1000
#define ANGULARVELOCITY_DELTA_TIME_MS		250			// gyro * dt =  angular * 4 / 1000

#define MOTIONSENSOR_PRINT_FORMAT_NICE 0
#define MOTIONSENSOR_PRINT_FORMAT_CSV  1

#define MOTIONSENSOR_PRINT_DATA_ALL 		0
#define MOTIONSENSOR_PRINT_DATA_NECESSARY	1
#define MOTIONSENSOR_PRINT_DATA_ACCEL_ANGLE	2

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

//there a three object of each sensor value: offset_vector; vector with concret data; filter buffer
//also a rawdata buffer
static acceleration_vector_t acceleration_vector;
static angularvelocity_vector_t angularvelocity_vector;
static int16_t	temperature;

static acceleration_vector_t acceleration_offset_vector;
static angularvelocity_vector_t angularvelocity_offset_vector; //TODO: replace with temp sensitiv solution

static mpu9150_motiondata_t raw_imudata;

// - FILTER
static uint8_t angularvelocity_filter_mask[] = {64, 1, 0, 0, 0, 0, 0, 0};
static filter_moving_generic_average_t angularvelocity_filter_x;
static filter_moving_generic_average_t angularvelocity_filter_y;
static filter_moving_generic_average_t angularvelocity_filter_z;

static uint8_t acceleration_filter_mask[] = {16, 8, 4, 2, 1, 0, 0, 0};
static filter_moving_generic_average_t acceleration_filter_x;
static filter_moving_generic_average_t acceleration_filter_y;
static filter_moving_generic_average_t acceleration_filter_z;

static uint8_t calibrate_zero_point_filter_mask[] = {1, 1, 1, 1, 1, 1, 1, 1};


// variables used in get_angle_y -> module internal - so print function can read them
static int16_t angle_y;
static int16_t acceleration_angle_y;
static double acceleration_angle_y_magnitude;

static int16_t angle_scalingfactor;
static uint8_t complementary_filter_angularvelocity_factor;
static uint8_t complementary_filter_acceleraton_factor;

static uint8_t printformat;
static uint8_t printdata;


/* * local function declarations * */
static void motionsensor_get_motiondata(void);

/* *** FUNCTION DEFINITIONS ************************************************* */

int16_t motionsensor_get_angle_y(void)
{
	int32_t fused_angles;
	int32_t accel_x_sqared;
	int32_t accel_z_sqared;


	//get sensordata data
	motionsensor_get_motiondata();

	//## get acceleration_angle of y
	//determine angle using acceleration vectors and atan and normalize
	//acceleration_angle_y = (int16_t)(atan2(acceleration_vector.x, -acceleration_vector.z) * NORMALIZATION_ACCELERATION_ANGLE);
	accel_x_sqared = (int32_t)acceleration_vector.x * (int32_t)acceleration_vector.x;
	accel_z_sqared = (int32_t)acceleration_vector.z * (int32_t)acceleration_vector.z;
	acceleration_angle_y_magnitude = (sqrt(accel_x_sqared + accel_z_sqared) / ((double)MOTIONSENSOR_ACCELERATION_1G)); //calculate magnitude and norm it to 1

	//calculate angle with acceleration using atan2 and respect magnitude
	acceleration_angle_y  = (int16_t)(
							(atan2(acceleration_vector.x , acceleration_vector.z)
							 / acceleration_angle_y_magnitude)
							 * NORMALIZATION_ACCELERATION_ANGLE);

	//##get angularvelocity of y
	//integrate angular velocity to angle over time (dt = 4 ms) (no normalization needed)
	angle_y += angularvelocity_vector.y / ANGULARVELOCITY_DELTA_TIME_MS; // gyro * dt =  angular * 4 / 1000

	//compensate gyro integral error using acceleration_angle
	//grad der qualität des winkels des beschleunigungssensors bewerten
	if(acceleration_angle_y_magnitude < 1.1 && acceleration_angle_y_magnitude > 0.9) {
		//fuse sensor data with complementary filter
		fused_angles  = ((int32_t)angle_y) * complementary_filter_angularvelocity_factor;
		fused_angles += ((int32_t)acceleration_angle_y) * complementary_filter_acceleraton_factor;
		angle_y = (int16_t)(fused_angles / MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE);
	}

	return angle_y * angle_scalingfactor;
}

void motionsensor_get_motiondata(void)
{
	//get raw data from imu
	mpu9150_read_motiondata(&raw_imudata);
	raw_imudata.acceleration.z = -raw_imudata.acceleration.z;	//invert z-axis to correct sign; yt heck?!

	//apply filter
	filter_moving_generic_average_put_element(&acceleration_filter_x, raw_imudata.acceleration.x);
	filter_moving_generic_average_put_element(&acceleration_filter_y, raw_imudata.acceleration.y);
	filter_moving_generic_average_put_element(&acceleration_filter_z, raw_imudata.acceleration.z);

	filter_moving_generic_average_put_element(&angularvelocity_filter_x, raw_imudata.angularvelocity.x);
	filter_moving_generic_average_put_element(&angularvelocity_filter_y, raw_imudata.angularvelocity.y);
	filter_moving_generic_average_put_element(&angularvelocity_filter_z, raw_imudata.angularvelocity.z);

	temperature = raw_imudata.temperature;

	//apply offset
	acceleration_vector.x = acceleration_filter_x.avg + acceleration_offset_vector.x;
	acceleration_vector.y = acceleration_filter_y.avg + acceleration_offset_vector.y;
	acceleration_vector.z = acceleration_filter_z.avg + acceleration_offset_vector.z;

	//TODO: Use temperature offset
	angularvelocity_vector.x = raw_imudata.angularvelocity.x + angularvelocity_offset_vector.x;
	angularvelocity_vector.y = raw_imudata.angularvelocity.y + angularvelocity_offset_vector.y;
	angularvelocity_vector.z = raw_imudata.angularvelocity.x + angularvelocity_offset_vector.z;
}

void motionsensor_printdata(void)
{
	//get raw data
	PORT_LEDS = 0xFF;
	motionsensor_get_angle_y();
	PORT_LEDS = 0x00;

	if(printdata == MOTIONSENSOR_PRINT_DATA_ACCEL_ANGLE) {
		printf("r:%6d %6d %6d - a:%6d m:%6f - %6d\n",
				acceleration_vector.x,
				acceleration_vector.z,
				angularvelocity_vector.y,
				acceleration_angle_y,
				acceleration_angle_y_magnitude,
				angle_y);

	} else if(printdata == MOTIONSENSOR_PRINT_DATA_ALL) {

		if (printformat == MOTIONSENSOR_PRINT_FORMAT_CSV) {

			printf("%d;%d;%d;%d;%d;%d;%d\n",
					acceleration_vector.x,
					acceleration_vector.y,
					acceleration_vector.z,
					angularvelocity_vector.x,
					angularvelocity_vector.y,
					angularvelocity_vector.z,
					temperature);

		} else { //printformat == MOTIONSENSOR_PRINTFORMAT_CSV

			printf("%6d %6d %6d %6d %6d %6d %6d\n",
					acceleration_vector.x,
					acceleration_vector.y,
					acceleration_vector.z,
					angularvelocity_vector.x,
					angularvelocity_vector.y,
					angularvelocity_vector.z,
					temperature);
		}
	} else { //printdata == MOTIONSENSOR_PRINT_DATA_NECESSARY

		if (printformat == MOTIONSENSOR_PRINT_FORMAT_CSV) {

			printf("%d;%d;%d;%d\n",
					acceleration_vector.x,
					acceleration_vector.z,
					angularvelocity_vector.y,
					temperature);

		} else { //printformat == MOTIONSENSOR_PRINTFORMAT_CSV

			printf("%6d %6d %6d %6d\n",
					acceleration_vector.x,
					acceleration_vector.z,
					angularvelocity_vector.y,
					temperature);
		}
	}
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


	//reset filters - use
	filter_moving_generic_average_init(&acceleration_filter_x, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_y, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_z, calibrate_zero_point_filter_mask, 0);

	filter_moving_generic_average_init(&angularvelocity_filter_x, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_x, calibrate_zero_point_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_x, calibrate_zero_point_filter_mask, 0);


	//loop: read rawdata + filter them...
	for(i = 0;i < FILTER_MOVING_GENERIC_WEIGHTS_COUNT * 8;i++) {
		motionsensor_get_motiondata();
	}

	//take current values and set them as offsets
	acceleration_offset_vector.x = -acceleration_vector.x;
	acceleration_offset_vector.y = -acceleration_vector.y;
	acceleration_offset_vector.z = (MOTIONSENSOR_ACCELERATION_1G - acceleration_vector.z);  //TOOD: define -1 G somewhere

	//TODO: replace with temp sensitiv solution
	angularvelocity_offset_vector.x = -angularvelocity_vector.x;
	angularvelocity_offset_vector.y = -angularvelocity_vector.y;
	angularvelocity_offset_vector.z = -angularvelocity_vector.z;

	//restore filter profiles
	filter_moving_generic_average_init(&acceleration_filter_x, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_y, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_z, acceleration_filter_mask, MOTIONSENSOR_ACCELERATION_1G);

	filter_moving_generic_average_init(&angularvelocity_filter_x, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_y, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_z, angularvelocity_filter_mask, 0);

	//read dummy data to fill up filters again
	for(i = 0;i < FILTER_MOVING_GENERIC_WEIGHTS_COUNT * 8;i++) {
		motionsensor_get_motiondata();
	}
}

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
	return angle_scalingfactor;
}

void motionsensor_set_angle_scalingfactor(uint16_t s)
{
	angle_scalingfactor = s;
}


uint8_t motionsensor_get_complementary_filter_ratio(void)
{
	//angularvelocity_factor == b; acceleration_factor == 1-b
	return complementary_filter_angularvelocity_factor;
}


//angularvelocity_factor == b; acceleration_factor == 1-b
void motionsensor_set_complementary_filter_ratio(uint8_t ratio)
{
	if(ratio > MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE) {
		ratio = MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE;
	}

	complementary_filter_angularvelocity_factor = ratio;
	complementary_filter_acceleraton_factor     = MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE - ratio;
}

void motionsensor_init(void)
{
	//init under laying hardware
	mpu9150_init();

	//init offset structures
	acceleration_offset_vector.x = 0;
	acceleration_offset_vector.y = 0;
	acceleration_offset_vector.z = 0;

	angularvelocity_offset_vector.x = 0;
	angularvelocity_offset_vector.y = 0;
	angularvelocity_offset_vector.z = 0;

	//init filters
	filter_moving_generic_average_init(&acceleration_filter_x, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_y, acceleration_filter_mask, 0);
	filter_moving_generic_average_init(&acceleration_filter_z, acceleration_filter_mask, 0);

	filter_moving_generic_average_init(&angularvelocity_filter_x, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_y, angularvelocity_filter_mask, 0);
	filter_moving_generic_average_init(&angularvelocity_filter_z, angularvelocity_filter_mask, 0);

	angle_scalingfactor = 1;

	complementary_filter_angularvelocity_factor = 100;
	complementary_filter_acceleraton_factor = 0;

	printformat = MOTIONSENSOR_PRINT_FORMAT_NICE;
	printdata   = MOTIONSENSOR_PRINT_DATA_ACCEL_ANGLE;

}
