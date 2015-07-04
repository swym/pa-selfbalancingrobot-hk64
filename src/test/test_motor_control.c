/*
 * test_motor_control.c
 *
 *  Created on: Nov 19, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "test_motor_control.h"

/* * system headers              * */
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <avr/interrupt.h>

/* * local headers               * */
#include "../motor_control.h"
#include "../leds.h"
#include "../uart.h"
#include "../timer.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

#define DELAY_MS 100
#define SPEED_START 0
#define SPEED_STOP	15

/* local function declarations  */
static void test_motor_control_run(void);
static void test_motor_control_init(void);

/* *** FUNCTION DEFINITIONS ************************************************** */

void test_motor_control(void)
{
	test_motor_control_init();
	test_motor_control_run();
}


void test_motor_control_init(void)
{
	uart_init(UART_BAUDRATE_115k);
	uart_init_stdio();

	motor_control_init(1);
	timer_init();
	leds_init();

	sei();

	printf("inited...\n");
}

void test_motor_control_run(void)
{
	int16_t mspeed = SPEED_START;
	uint8_t i;

	motor_control_speed_t speed;
	motor_control_speed_t cur_speed;
	speed.motor_1 = mspeed;
	speed.motor_2 = mspeed;
	leds_set(mspeed);

//	for(;;) {
//		leds_color(LEDS_COLOR_RED);
//
//		motor_control_prepare_new_speed(&speed);
//		motor_control_set_new_speed();
//		motor_control_get_current_speed(&cur_speed);
//
//		printf("pwm:%5d m1:%5d m2:%5d\n", 255, cur_speed.motor_1, cur_speed.motor_2);
//
//	}





//	while(mspeed < SPEED_STOP) {
//		leds_color(LEDS_COLOR_CYAN);
//
//		motor_control_prepare_new_speed(&speed);
//		motor_control_set_new_speed();
//		motor_control_get_current_speed(&cur_speed);
//
//		for(i = 0; i < 30;i++) {
//			_delay_ms(DELAY_MS);
//			printf("pwm:%5d m1:%5d m2:%5d\n", mspeed, cur_speed.motor_1, cur_speed.motor_2);
//		}
//
//		mspeed++;
//
//		speed.motor_1 = mspeed;
//		speed.motor_2 = mspeed;
//		leds_set(mspeed);
//
//	}

	mspeed = -SPEED_START;
	speed.motor_1 = mspeed;
	speed.motor_2 = mspeed;

	while(mspeed > -SPEED_STOP) {
		leds_color(LEDS_COLOR_MAGENTA);

		motor_control_prepare_new_speed(&speed);
		motor_control_set_new_speed();
		motor_control_get_current_speed(&cur_speed);

		for(i = 0; i < 30;i++) {
			_delay_ms(DELAY_MS);
			printf("pwm:%5d m1:%5d m2:%5d\n", mspeed, cur_speed.motor_1, cur_speed.motor_2);
		}

		mspeed--;

		speed.motor_1 = mspeed;
		speed.motor_2 = mspeed;
		leds_set(abs(mspeed));
	}


	speed.motor_1 = 0;
	speed.motor_2 = 0;
	motor_control_prepare_new_speed(&speed);
	motor_control_set_new_speed();
	leds_color(LEDS_COLOR_BLACK);
}

/*

void test_motor_control_set_different_speeds(void)
{



	motor_contol_speed_t speed;
	motor_contol_speed_t printf_speed;

	//init
	md25_set_mode(MD25_MODE_SIGNED_SPEED);

	printf("test_motor_control_set_different_speeds(void)");
	printf("init\n");

	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	//1:neue Geschwindigkeit einstellen
	//2:delay
	//goto 1
	speed.motor_1 = 0;
	speed.motor_2 = 0;

	motor_control_prepare_new_speed(&speed);
	motor_control_set_new_speed();

	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);


	_delay_ms(2000.0);


	// SPEED 1
	speed.motor_1 = 100;
	speed.motor_2 = 100;

	motor_control_prepare_new_speed(&speed);

	_delay_ms(2000.0);

	motor_control_set_new_speed();
	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	_delay_ms(2000.0);


	// SPEED 1
	speed.motor_1 = 0;
	speed.motor_2 = 0;

	motor_control_prepare_new_speed(&speed);

	_delay_ms(1000.0);

	motor_control_set_new_speed();
	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	_delay_ms(2000.0);


	// SPEED 1
	speed.motor_1 = 0;
	speed.motor_2 = -100;

	motor_control_prepare_new_speed(&speed);

	_delay_ms(1000.0);

	motor_control_set_new_speed();
	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	_delay_ms(2000.0);

	// SPEED 1
	speed.motor_1 = -100;
	speed.motor_2 = 0;

	motor_control_prepare_new_speed(&speed);

	_delay_ms(1000.0);

	motor_control_set_new_speed();
	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	_delay_ms(2000.0);

	// SPEED 1
	speed.motor_1 = -100;
	speed.motor_2 = 100;

	motor_control_prepare_new_speed(&speed);

	_delay_ms(1000.0);

	motor_control_set_new_speed();
	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	_delay_ms(2000.0);

	// SPEED 1
	speed.motor_1 = 0;
	speed.motor_2 = 0;

	motor_control_prepare_new_speed(&speed);

	_delay_ms(1000.0);

	motor_control_set_new_speed();
	motor_control_get_current_speed(&printf_speed);
	printf("current: M1 = %d , M2 = %d\n",printf_speed.motor_1, printf_speed.motor_2);

	_delay_ms(2000.0);

	printf("stop\n");
}

*/
