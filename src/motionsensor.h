/*
 * motionsensor.h
 *
 *  Created on: Jun 28, 2014
 *      Author: alexandermertens
 */


#ifndef MOTIONSENSOR_H_
#define MOTIONSENSOR_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} rotation_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} acceleration_t;

typedef struct {
	rotation_t rotation;
	acceleration_t acceleration;
} motion_t;

/* * external objects            * */
extern void motionsensor_init(void);
extern void motionsensor_set_zero_point();

extern double motionsensor_get_position();

//TODO: make static when working
extern void motionsensor_get_current_rotation(rotation_t *rotation);
extern void motionsensor_get_current_acceleration(acceleration_t *acceleration);

/* * external functions          * */

#endif /* MOTIONSENSOR_H_ */
