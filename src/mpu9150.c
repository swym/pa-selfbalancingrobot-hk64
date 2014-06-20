/*
 * mpu9150.c
 *
 *  Created on: Jun 20, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "mpu9150.h"
/* * system headers              * */

/* * local headers               * */
#include "lib/twi_master.h"

/* *** DEFINES ************************************************************** */
#define MPU9150_TWI_ADDRESS			0x68

/* *** REGISTER NAMING *** */
#define MPU9150_REGISTER_WHO_AM_I	0x75
#define MPU9150_REGISTER_POSITON	0

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */

/**
 * This register is used to verify the identity of the device. The contents of
 * WHO_AM_I are the upper 6 bits of the MPU-9150’s 7-bit I2C address. The least
 * significant bit of the MPU-1950’s I2C address is determined by the value of
 * the AD0 pin. The value of the AD0 pin is not reflected in this register.
 *
 * The default value of the register is 0x68. Bits 0 and 7 are reserved.
 * (Hard coded to 0)
 * @return 0x68
 */
uint8_t mpu9150_get_who_am_i()
{
	uint8_t register_value = 0xFF;

	register_value = twi_master_read_register(MPU9150_TWI_ADDRESS,
											  MPU9150_REGISTER_WHO_AM_I);

	return 0x7E & register_value;
}
