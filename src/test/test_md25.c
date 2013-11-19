/*
 * test_md25.c
 *
 *  Created on: Nov 15, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "test_md25.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <util/delay.h>

#include "../md25.h"
/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */
void test_md25_encoders(void)
{
	md25_cmd_reset_encoders();

	md25_set_mode(MD25_MODE_SIGNED_SPEED);

	md25_set_speed(0,0);
	md25_set_motor1_speed(33);

	for(;;) {
		PORTC ^= 1;

		printf("encoder M1: %ld ", md25_get_motor_encoder(MD25_MOTOR1));
		printf("encoder M2: %ld\n", md25_get_motor_encoder(MD25_MOTOR2));
	}
}

void test_md25_volts_and_currents(void)
{
	md25_cmd_reset_encoders();

	md25_set_mode(MD25_MODE_SIGNED_SPEED);
	md25_set_speed(-10, 10);

	for(;;) {
		PORTC ^= 1;
		printf("V: %d ",md25_get_battery_volts());
		printf("A(M1): %d ",md25_get_motor_current(MD25_MOTOR1));
		printf("A(M2): %d\n",md25_get_motor_current(MD25_MOTOR2));
	}

}


void test_md25_mode(void)
{
	uint8_t i = 0;

	md25_cmd_reset_encoders();

	md25_set_mode(MD25_MODE_SIGNED_SPEED);
	md25_set_acceleration_rate(MD25_ACCELERATION_MAX);
	md25_set_speed(0, 0);



	/* MODE 1 */
	md25_set_mode(MD25_MODE_UNSIGNED_SPEED);

	for(i = 0;i < 32;i++) {
		md25_set_speed(64, 64);
		_delay_ms(100.0);
	}

	/* MODE 2 */
	md25_set_mode(MD25_MODE_SIGNED_SPEED);

	for(i = 0;i < 32;i++) {
		md25_set_speed(64, 64);
		_delay_ms(100.0);
	}

	/* MODE 3 */
	md25_set_mode(MD25_MODE_SIGNED_TURN);

	for(i = 0;i < 32;i++) {
		md25_set_speed(64, 64);
		_delay_ms(100.0);
	}

	/* MODE 4 */
	md25_set_mode(MD25_MODE_UNSIGNED_TURN);

	for(i = 0;i < 32;i++) {
		md25_set_speed(64, 64);
		_delay_ms(100.0);
	}
}

void test_md25_acceleration(void)
{
	uint8_t i = 0;

	printf("start test_md25_acceleration(void)\n");


	printf("md25_set_speed(0, 0)\n");
	md25_set_mode(MD25_MODE_SIGNED_SPEED);
	md25_cmd_enable_speed_regulation(true);
	md25_set_speed(0, 0);

	printf("md25_set_acceleration_rate(MD25_ACCELERATION_MIN)\n");
	md25_set_acceleration_rate(MD25_ACCELERATION_MIN);

	printf("md25_set_speed(100, 0);\n");
	for(i = 0;i < 10;i++) {
		md25_set_speed(100, 0);
		_delay_ms(1000.0);
	}

	printf("md25_cmd_enable_speed_regulation(false);\n");
	md25_cmd_enable_speed_regulation(false);

	printf("md25_set_speed(0, 0)\n");
	md25_set_speed(0, 0);

	_delay_ms(5000.0);

	printf("md25_set_speed(100, 0)\n");
	for(i = 0;i < 5;i++) {
		md25_set_speed(100, 0);
		_delay_ms(1000.0);
	}
}
