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
#include "motionsensor.h"

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */

/* * external objects            * */

/* * external functions          * */

extern void     mpu9150_init();
extern void     mpu9150_read_acceleration(acceleration_t*);

extern void     mpu9150_read_angularvelocity(angularvelocity_t *);

extern int16_t	mpu9150_read_angularvelocity_x(void);
extern int16_t	mpu9150_read_angularvelocity_y(void);
extern int16_t	mpu9150_read_angularvelocity_z(void);

extern uint8_t  mpu9150_get_who_am_i(void);

extern uint8_t mpu9150_get_int_status(void);

#endif /* MPU9150_H_ */
