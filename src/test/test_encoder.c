/*
 * test_encoder.c
 *
 *  Created on: Mar 4, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_encoder.h"

/* * system headers              * */
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

/* * local headers               * */
#include "../l6205.h"
#include "../timer.h"
#include "../leds.h"
#include "../encoder.h"
#include "../uart.h"

/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */
volatile timer_slot_t timer_current_majorslot;
volatile timer_slot_t timer_current_minorslot;

/* * local objects               * */

/* * local function declarations * */
void test_encoder_init(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_encoder_run(void)
{
	int16_t timer_cnt = 0;

	int16_t delta_m1 = 0;
	int16_t delta_m2 = 0;

	int16_t way_m1 = 0;
	int16_t way_m2 = 0;

	int16_t set_speed = 0;

	uint8_t direction = 1;


	test_encoder_init();


	for(;;) {

		//every timer irq ...
		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			delta_m1 = encoder_read_delta(ENCODER_M1);
			way_m1 += delta_m1;

			delta_m2 = encoder_read_delta(ENCODER_M2);
			way_m2 += delta_m2;

			printf("set_speed: %4d d_m1: %4d d_m2: %4d w_m1: %4d w_m2: %4d\n",
					set_speed, delta_m1, delta_m2, way_m1, way_m2);

			//each fice seconds inc speed
			if(timer_cnt >= 5000 && direction == 1) {
				timer_cnt = 0;

				set_speed = 100;
				l6205_set_speed(MOTOR_1, set_speed);
				l6205_set_speed(MOTOR_2, set_speed);

				direction = 0;
			}

			if(timer_cnt >= 5000 && direction == 0) {
				timer_cnt = 0;

				set_speed = -100;
				l6205_set_speed(MOTOR_1, set_speed);
				l6205_set_speed(MOTOR_2, set_speed);

				direction = 1;
			}

			timer_cnt++;
		}
	}
}

void test_encoder_init(void)
{
	timer_init();
	l6205_init(1);
	encoder_init();
	init();

	sei();

	printf("inited...\n");
}
