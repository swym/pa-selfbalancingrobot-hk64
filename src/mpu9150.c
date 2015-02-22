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
#define MPU9150_TWI_ADDRESS				0x68
#define MPU9150_MAG_TWI_ADDRESS			0x0C

/* *** REGISTER NAMING *** */
#define MPU9150_REGISTER_CONFIG			0x1A
#define MPU9150_REGISTER_SMPRT_DIV		0x19
#define MPU9150_REGISTER_GYRO_CONFIG	0x1B
#define MPU9150_REGISTER_ACCEL_CONFIG	0x1C

#define MPU9150_REGISTER_INT_PIN_CFG	0x37
#define MPU9150_REGISTER_INT_ENABLE		0x38
#define MPU9150_REGISTER_INT_STATUS		0x3A

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


void mpu9150_read_motiondata(mpu9150_motiondata_t * motiondata)
{
	uint8_t temp_data;

	twi_send_buffer[0] = MPU9150_REGISTER_ACCEL_XOUT_H;
	twi_master_set_ready();

	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 14);

	temp_data = twi_receive_buffer[0];
	motiondata->acceleration.x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	motiondata->acceleration.x = (uint16_t)(motiondata->acceleration.x | temp_data);

	temp_data = twi_receive_buffer[2];
	motiondata->acceleration.y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[3];
	motiondata->acceleration.y = (uint16_t)(motiondata->acceleration.y | temp_data);

	temp_data = twi_receive_buffer[4];
	motiondata->acceleration.z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[5];
	motiondata->acceleration.z = (uint16_t)(motiondata->acceleration.z | temp_data);

	temp_data = twi_receive_buffer[6];
	motiondata->temp = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[7];
	motiondata->temp = (uint16_t)(motiondata->temp | temp_data);

	temp_data = twi_receive_buffer[8];
	motiondata->angularvelocity.x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[9];
	motiondata->angularvelocity.x = (uint16_t)(motiondata->angularvelocity.x | temp_data);

	temp_data = twi_receive_buffer[10];
	motiondata->angularvelocity.y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[11];
	motiondata->angularvelocity.y = (uint16_t)(motiondata->angularvelocity.y | temp_data);

	temp_data = twi_receive_buffer[12];
	motiondata->angularvelocity.z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[13];
	motiondata->angularvelocity.z = (uint16_t)(motiondata->angularvelocity.z | temp_data);
}

void mpu9150_read_angularvelocity(mpu9150_angularvelocity_vector_t * angularvelocity)
{
	uint8_t temp_data;

	twi_send_buffer[0] = MPU9150_REGISTER_GYRO_XOUT_H;
	twi_master_set_ready();

	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 6);

	temp_data = twi_receive_buffer[0];
	angularvelocity->x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	angularvelocity->x = (uint16_t)(angularvelocity->x | temp_data);

	temp_data = twi_receive_buffer[2];
	angularvelocity->y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[3];
	angularvelocity->y = (uint16_t)(angularvelocity->y | temp_data);

	temp_data = twi_receive_buffer[4];
	angularvelocity->z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[5];
	angularvelocity->z = (uint16_t)(angularvelocity->z | temp_data);
}

mpu9150_angularvelocity mpu9150_read_angularvelocity_x(void)
{
	uint8_t temp_data;
	mpu9150_angularvelocity rotation_x;

	twi_send_buffer[0] = MPU9150_REGISTER_GYRO_XOUT_H;
	twi_master_set_ready();

	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 2);

	temp_data = twi_receive_buffer[0];
	rotation_x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	rotation_x = (uint16_t)(rotation_x | temp_data);

	return rotation_x;
}

mpu9150_angularvelocity mpu9150_read_angularvelocity_y(void)
{
	uint8_t temp_data;
	mpu9150_angularvelocity rotation_y;

	twi_send_buffer[0] = MPU9150_REGISTER_GYRO_YOUT_H;
	twi_master_set_ready();

	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 2);

	temp_data = twi_receive_buffer[0];
	rotation_y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	rotation_y = (uint16_t)(rotation_y | temp_data);

	return rotation_y;
}

mpu9150_angularvelocity mpu9150_read_angularvelocity_z(void)
{
	uint8_t temp_data;
	mpu9150_angularvelocity rotation_z;

	twi_send_buffer[0] = MPU9150_REGISTER_GYRO_ZOUT_H;
	twi_master_set_ready();

	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 2);

	temp_data = twi_receive_buffer[0];
	rotation_z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	rotation_z = (uint16_t)(rotation_z | temp_data);

	return rotation_z;
}

void mpu9150_read_acceleration(mpu9150_acceleration_vector_t * acceleration)
{

	uint8_t temp_data;

	twi_send_buffer[0] = MPU9150_REGISTER_ACCEL_XOUT_H;


	twi_master_set_ready();
	twi_send_data(MPU9150_TWI_ADDRESS, 1);
	twi_receive_data(MPU9150_TWI_ADDRESS, 6);

	temp_data = twi_receive_buffer[0];
	acceleration->x = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[1];
	acceleration->x = (uint16_t)(acceleration->x | temp_data);

	temp_data = twi_receive_buffer[2];
	acceleration->y = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[3];
	acceleration->y = (uint16_t)(acceleration->y | temp_data);

	temp_data = twi_receive_buffer[4];
	acceleration->z = (uint16_t)(temp_data << 8);
	temp_data = twi_receive_buffer[5];
	acceleration->z = (uint16_t)(acceleration->z | temp_data);
	acceleration->z = -acceleration->z;
}

uint8_t mpu9150_get_int_status(void)
{
	return twi_master_read_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_INT_STATUS);
}

void mpu9150_init()
{
	//Device Reset
	//set clocksource to x gyro as discripted in manual
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1, 0x41);

	//GYRO resolution
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_GYRO_CONFIG, 0x00);

	//ACCEL resolution
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_ACCEL_CONFIG, 0x00);

	//Disable Data Interrupt
	//twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_INT_ENABLE, 0x00);
	//Enable Data IRQ
//	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_INT_ENABLE, 0x01);

	//Set Samplerate devider from 8 kHz down to 250 Hz
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_SMPRT_DIV, 0x1F);

	//Set Digital Low Pass Filter to 188 Hz
//	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_CONFIG, 0x01);

	//enable bypass mode for accessing compass directly
//	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_INT_PIN_CFG, 0x02);

	//enable magnetometer
//	twi_master_write_register(MPU9150_MAG_TWI_ADDRESS, 0x0A, 0x01);

	//wake up (disable sleep)
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1, 0x01);
}
