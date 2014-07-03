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
#define MPU9150_REGISTER_GYRO_CONFIG	0x1B
#define MPU9150_REGISTER_ACCEL_CONFIG	0x1C

#define MPU9150_REGISTER_ACCEL_XOUT_H	0x3B

#define MPU9150_REGISTER_GYRO_XOUT_H	0x43
#define MPU9150_REGISTER_GYRO_XOUT_L	0x44
#define MPU9150_REGISTER_GYRO_YOUT_H	0x45
#define MPU9150_REGISTER_GYRO_YOUT_L	0x46
#define MPU9150_REGISTER_GYRO_ZOUT_H	0x47
#define MPU9150_REGISTER_GYRO_ZOUT_L	0x48

#define MPU9150_REGISTER_PWR_MGMT_1		0x6B

#define MPU9150_REGISTER_WHO_AM_I		0x75

/* *** BIT NAMING *** */
/* MPU9150_REGISTER_GYRO_CONFIG 0x1B */
#define MPU9150_BIT_FS_SEL_0		3
#define MPU9150_BIT_FS_SEL_1		4


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
uint8_t mpu9150_get_who_am_i(void)
{
	uint8_t register_value = 0xFF;

	register_value = twi_master_read_register(MPU9150_TWI_ADDRESS,
											  MPU9150_REGISTER_WHO_AM_I);

	return 0x7E & register_value;
}

/**
 * This method is used to configure the gyroscopes’ full scale range.
 * Valid values for range (uint8_t):
 * range (uint16_t)  | Full Scale Range
 * :--------------:  | : -----------------------------:
 * 250               | ± 250 °/s
 * 500               | ± 500 °/s
 * 1000              | ± 1000 °/s
 * 2000              | ± 2000 °/s

 * @param range - New range value
 * @return boolean - return true, if param is valid and write sequence to the
 * 					 sensor is completed.
 * 					 return false, if param is invalid. Then no write sqquence
 * 					 was performed. so
 */
//bool mpu9150_gyro_set_range(uint16_t range)
//{
//	if(range == 250 || range == 500 || range == 1000 || range == 2000) {
//
//		uint8_t value = 0;
//
//		/* read register and delete old value */
//		value = twi_master_read_register(MPU9150_TWI_ADDRESS,
//										 MPU9150_REGISTER_GYRO_CONFIG);
//		value &= ~(1<<MPU9150_BIT_FS_SEL_1 | 1<<MPU9150_BIT_FS_SEL_0); /* (value &= 0b11100111;)*/
//
//
//		if(range == 250) {
//			/* if range == 2g; set no bits */
//		} else if(range == 500) {
//			value |= (1<<MPU9150_BIT_FS_SEL_0);
//		} else if(range == 1000) {
//			value |= (1<<MPU9150_BIT_FS_SEL_1);
//		} else if(range == 2000) {
//			value |= (1<<MPU9150_BIT_FS_SEL_1 | 1<<MPU9150_BIT_FS_SEL_0);
//		}
//
//		twi_master_write_register(MPU9150_TWI_ADDRESS,
//				MPU9150_REGISTER_GYRO_CONFIG,
//				value);
//
//		return true;
//	} else {
//		return false;
//	}
//}

/**
 * returns current range of the gyro
 * @return
 */
//
//uint16_t mpu9150_gyro_get_range(void)
//{
//	uint8_t register_value;
//	uint16_t return_value;
//
//	/* read register and delete all non range-bits*/
//	register_value = twi_master_read_register(MPU9150_TWI_ADDRESS,
//											  MPU9150_REGISTER_GYRO_CONFIG);
//	register_value &= (1<<MPU9150_BIT_FS_SEL_1 | 1<<MPU9150_BIT_FS_SEL_0); /* (value &= 0b00011000;)*/
//
//	/*convert to human readable values */
//	if(register_value == (1<<MPU9150_BIT_FS_SEL_1 | 1<<MPU9150_BIT_FS_SEL_0)) {
//		return_value = 2000;
//	} else if(register_value == 1<<MPU9150_BIT_FS_SEL_0) {
//		return_value = 1000;
//	} else if (register_value == 1<<MPU9150_BIT_FS_SEL_1) {
//		return_value = 500;
//	} else {
//		/* if no bit is set; range == 2 */
//		return_value = 250;
//	}
//	return return_value;
//}

extern void mpu9150_read_rotation(rotation_t* rotation_vector)
{
	uint8_t temp_data;

	twi_send_buffer[0] = MPU9150_REGISTER_GYRO_XOUT_H;

	twi_master_set_ready();
	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 6);

	temp_data = twi_receive_buffer[0];
	rotation_vector->x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	rotation_vector->x = (uint16_t)(rotation_vector->x | temp_data);

	temp_data = twi_receive_buffer[2];
	rotation_vector->y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[3];
	rotation_vector->y = (uint16_t)(rotation_vector->y | temp_data);

	temp_data = twi_receive_buffer[4];
	rotation_vector->z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[5];
	rotation_vector->z = (uint16_t)(rotation_vector->z | temp_data);
}

extern void mpu9150_read_acceleration(acceleration_t* acceleration_vector)
{
	uint8_t temp_data;

	twi_send_buffer[0] = MPU9150_REGISTER_ACCEL_XOUT_H;

	twi_master_set_ready();
	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 6);

	temp_data = twi_receive_buffer[0];
	acceleration_vector->x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	acceleration_vector->x = (uint16_t)(acceleration_vector->x | temp_data);

	temp_data = twi_receive_buffer[2];
	acceleration_vector->y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[3];
	acceleration_vector->y = (uint16_t)(acceleration_vector->y | temp_data);

	temp_data = twi_receive_buffer[4];
	acceleration_vector->z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[5];
	acceleration_vector->z = (uint16_t)(acceleration_vector->z | temp_data);
	acceleration_vector->z = -acceleration_vector->z;
}

void mpu9150_init()
{
	//set clocksource to x gyro as discripted in manual
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1, 0x41);

	//GYRO resolution
	//twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_GYRO_CONFIG, 0x18);

	//ACCEL resolution
	//twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_GYRO_CONFIG, 0x18);

	//wake up (disable sleep)
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1, 0x01);
}
