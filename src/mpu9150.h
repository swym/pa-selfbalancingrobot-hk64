/*
 * mpu9150.h
 *
 *  Created on: Jun 20, 2014
 *      Author: alexandermertens
 */


#ifndef MPU9150_H_
#define MPU9150_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>
#include <stdbool.h>

/* * local headers               * */
//#include "motionsensor.h"

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */

typedef int16_t mpu9150_acceleration_t;
typedef int16_t mpu9150_angularvelocity;
typedef int16_t mpu9150_temperature_t;

typedef struct {
	mpu9150_acceleration_t x;
	mpu9150_acceleration_t y;
	mpu9150_acceleration_t z;
} mpu9150_acceleration_vector_t;

typedef struct {
	mpu9150_angularvelocity x;
	mpu9150_angularvelocity y;
	mpu9150_angularvelocity z;
} mpu9150_angularvelocity_vector_t;

typedef struct {
	mpu9150_acceleration_vector_t acceleration;
	mpu9150_angularvelocity_vector_t angularvelocity;
	mpu9150_temperature_t temperature;
} mpu9150_data_t;

/* * external objects            * */

/* * external functions          * */

extern void mpu9150_init();
extern void	mpu9150_read_motiondata(mpu9150_data_t * motiondata);

extern void mpu9150_read_acceleration(mpu9150_acceleration_vector_t * acceleration);
extern void mpu9150_read_angularvelocity(mpu9150_angularvelocity_vector_t * angularvelocity);
extern uint8_t mpu9150_get_who_am_i(void);
extern uint8_t mpu9150_get_int_status(void);

#endif /* MPU9150_H_ */
