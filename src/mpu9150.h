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

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */

/* * external objects            * */

/* * external functions          * */
extern uint8_t mpu9150_get_who_am_i();

extern uint8_t mpu9150_get_rotation_x();

extern bool mpu9150_gyro_set_range(uint16_t); /* uint16_t range */
extern uint16_t mpu9150_gyro_get_range(void);

extern void mpu9150_init();

#endif /* MPU9150_H_ */
