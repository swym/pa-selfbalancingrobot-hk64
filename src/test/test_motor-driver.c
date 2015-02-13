/*
 * test_motor-driver.c
 *
 *  Created on: Feb 12, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */

/* * system headers              * */
#include <stdio.h>
#include <stdbool.h>

#include <util/delay.h>
#include <avr/interrupt.h>

/* * local headers               * */
#include "../l6205pd.h"
#include "../timer.h"
#include "../common.h"
#include "../lib/uart.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
static uint16_t motor_speed_timer;
static uint8_t led_value;

/* * local function declarations * */
void test_motor_driver_init(void);


/* *** FUNCTION DEFINITIONS ************************************************* */


void test_motor_driver(void)
{
	test_motor_driver_init();

	bool set_speed = true;

	for(;;) {

		if(timer_slot_0) {

			if(set_speed) {

				motor_set_speed(MOTOR_1, 255);

				set_speed = false;
			}
		}

			/*

			motor_speed_timer++;

			if(motor_speed_timer == 1) {


				printf("step1\n");
				PORTA = led_value++;

				motor_set_speed(MOTOR_1, 128);
				motor_set_speed(MOTOR_2, 128);
			}

			if(motor_speed_timer == 20480) {

				printf("step2\n");
				PORTA = led_value++;

				motor_set_speed(MOTOR_1, 0);
				motor_set_speed(MOTOR_2, 0);
			}

			if(motor_speed_timer == 40960) {

				printf("step3\n");
				PORTA = led_value++;

				motor_set_speed(MOTOR_1, 255);
				motor_set_speed(MOTOR_2, 255);
			}


			if(motor_speed_timer > 40960) {


				motor_speed_timer = 0;
			}

		}

		*/

		motor_update_pwm();
	}
}

void test_motor_driver_init(void)
{
	timer_init();
	init_motors();

	UART_init(57600);

	DDRA = 0xFF;

	sei();

	printf("inited...\n");
}
