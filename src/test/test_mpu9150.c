/*
 * test_mpu9150.c
 *
 *  Created on: Jun 20, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_mpu9150.h"


/* * system headers              * */
#include <stdlib.h>
#include <stdio.h>

#include <util/delay.h>

/* * local headers               * */
#include "../lib/uart.h"
#include "../lib/twi_master.h"
#include "../lib/rfm12.h"

#include "../simplex-protocol.h"
#include "../accelerationsensor.h"

#include "../test/test_twi_master.h"

#include "../mpu9150.h"


/* *** DEFINES ************************************************************** */
#define MPU9150_TWI_ADDRESS				0x68

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

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */
static void test_mpu9150_init();
static void test_mpu9150_with_rfm12();
static void test_mpu9150_with_uart();

/* *** FUNCTION DEFINITIONS ************************************************* */
static void test_mpu9150_with_rfm12(void)
{
	simplex_protocol_init();
	rfm12_init();

	_delay_ms(500);  //little delay for the rfm12 to initialize properly
	printf("rfm12 inited.\n");

	PORTC |= _BV(1);

	rotation_t rot_vector;
	acceleration_t accel_vector;

	uint8_t data_len = 6;
	uint8_t data[6];

	printf("start sending angular velocity ...\n");

	while(true) {

		mpu9150_read_rotation(&rot_vector);
		mpu9150_read_acceleration(&accel_vector);

		data[0]  = (uint8_t)(accel_vector.x >> 8);
		data[1]  = (uint8_t)(accel_vector.x & 0x00FF);

		data[2]  = (uint8_t)(accel_vector.y >> 8);
		data[3]  = (uint8_t)(accel_vector.y & 0x00FF);

		data[4]  = (uint8_t)(accel_vector.z >> 8);
		data[5]  = (uint8_t)(accel_vector.z & 0x00FF);

		simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
							 data_len,
							 data);

		simplex_protocol_tick();

		_delay_ms(20);
	}
}

static void test_mpu9150_with_uart()
{
	printf("test_mpu9150_with_uart()\n");

	rotation_t rot_vector;
	acceleration_t accel_vector;

	while(true) {

		PORTC ^= _BV(PC7);

		mpu9150_read_acceleration(&accel_vector);
		mpu9150_read_rotation(&rot_vector);

		printf("%d:%d:%d:%d:%d:%d:\n",
				accel_vector.x, accel_vector.y, accel_vector.z,
				rot_vector.x,rot_vector.y, rot_vector.z);

		_delay_ms(100);
	}
}

void test_mpu9150_run()
{
	test_mpu9150_init();
	printf("test_mpu9150_inited\n");

	//test_mpu9150_with_rfm12();
	test_mpu9150_with_uart();
}

void test_mpu9150_init()
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */

	UART_init(38400);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */

	PORTC |= _BV(0);

	sei();

	mpu9150_init();
}
