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

#include "../motionsensor.h"
#include "../mpu9150.h"
#include "../timer.h"

#include "../leds.h"




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
#define MPU9150_REGISTER_SMPRT_DIV		0x19

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
volatile timer_slot_t timer_current_majorslot;
volatile timer_slot_t timer_current_minorslot;

/* * local function declarations * */
static void test_mpu9150_init();
static void test_mpu9150_with_uart();

/* *** FUNCTION DEFINITIONS ************************************************* */

static void test_mpu9150_with_uart()
{
	printf("test_mpu9150_with_uart()\n");

	mpu9150_motiondata_t raw_data;
	uint8_t tmp;

	_delay_ms(100);

	tmp = twi_master_read_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_PWR_MGMT_1);
	printf("PWR_MGMT_1: %X\n", tmp);

	tmp = twi_master_read_register(MPU9150_TWI_ADDRESS, MPU9150_REGISTER_SMPRT_DIV);
	printf("SMPRT_DIV: %X\n", tmp);

	tmp = twi_master_read_register(MPU9150_TWI_ADDRESS, 0x1B);
	printf("GYRO_CONFIG: %X\n", tmp);

	tmp = twi_master_read_register(MPU9150_TWI_ADDRESS, 0x1C);
	printf("ACCEL_CONFIG: %X\n", tmp);


	_delay_ms(2000);

	while(true) {

		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = false;

			PORT_LEDS ^= _BV(0);

			mpu9150_read_motiondata(&raw_data);

			printf("%7d%7d%7d%7d%7d%7d%7d\n",
					raw_data.acceleration.x,
					raw_data.acceleration.y,
					raw_data.acceleration.z,
					raw_data.angularvelocity.x,
					raw_data.angularvelocity.y,
					raw_data.angularvelocity.z,
					raw_data.temperature);
		}
	}
}

void test_mpu9150_run()
{
	test_mpu9150_init();
	printf("test_mpu9150_inited\n");


	test_mpu9150_with_uart();
}

void test_mpu9150_init()
{
	leds_init();							/* Data Direction Register der LEDs als Ausgang definieren */

	UART_init();							/* Init UART mit 57600 baud */
	sei();
	twi_master_init(TWI_TWBR_VALUE_400);	/* Init TWI/I2C Schnittstelle */
	timer_init();
	mpu9150_init();
}
