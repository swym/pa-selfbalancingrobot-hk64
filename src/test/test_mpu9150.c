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

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} rotation_t;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} acceleration_t;

/* * local function declarations * */
static void test_mpu9150_init();
static void test_mpu9150_read_rotation(rotation_t *);
static void test_mpu9150_read_acceleration(acceleration_t*);

/* *** FUNCTION DEFINITIONS ************************************************* */
void test_mpu9150_run(void)
{
	test_mpu9150_init();

	PORTC |= _BV(1);

	rotation_t rot_vector;
	acceleration_t accel_vector;

	uint8_t data_len = 6;
	uint8_t data[6];

	printf("start sending angular velocity ...\n");

	while(true) {



		test_mpu9150_read_rotation(&rot_vector);
		test_mpu9150_read_acceleration(&accel_vector);
//		printf("%d:%d:%d:%d:%d:%d:\n",
//				accel_vector.x, accel_vector.y, accel_vector.z,
//				rot_vector.x,rot_vector.y, rot_vector.z);
		PORTC|= _BV(7);
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

		simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
							 data_len,
							 data);

		simplex_protocol_tick();

		simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
							 data_len,
							 data);

		simplex_protocol_tick();

		PORTC &= ~_BV(7);
		_delay_ms(20);
	}
}

static void test_mpu9150_read_rotation(rotation_t* rotation_vector)
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

static void test_mpu9150_read_acceleration(acceleration_t* acceleration_vector)
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
}



void test_mpu9150_init()
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */

	UART_init(38400);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */

	PORTC |= _BV(0);

	simplex_protocol_init();
	rfm12_init();

	_delay_ms(500);  //little delay for the rfm12 to initialize properly
	printf("rfm12 inited.\n");

	sei();


	//set clocksource to x gyro as discripted in manual
	printf("c%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_PWR_MGMT_1));
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1, 0x41);
	printf("c%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_PWR_MGMT_1));

	//GYRO resolution
	printf("g%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_GYRO_CONFIG));
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_GYRO_CONFIG, 0x18);
	printf("g%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_GYRO_CONFIG));

	//ACCEL resolution
	printf("a%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_ACCEL_CONFIG));
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_GYRO_CONFIG, 0x18);
	printf("a%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_ACCEL_CONFIG));

	//wake up (disable sleep)
	PORTC ^= _BV(6);
	printf("s%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_PWR_MGMT_1));
	twi_master_write_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1, 0x01);
	printf("s%c", twi_master_read_register(MPU9150_TWI_ADDRESS,MPU9150_REGISTER_PWR_MGMT_1));

}
