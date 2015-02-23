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
#include "../timer.h"
#include "../l6205.h"
#include "../lib/uart.h"


/* *** DEFINES ************************************************************** */

#define TICKS_TO_EVENT 		250

/* *** DECLARATIONS ********************************************************* */

volatile timer_slot_t timer_current_minorslot;
volatile timer_slot_t timer_current_majorslot;

/* * local type and constants    * */

/* * local objects               * */
static uint16_t tickcnt;
static uint8_t  event;

/* * local function declarations * */
void test_motor_driver_init(void);


/* *** FUNCTION DEFINITIONS ************************************************* */


void test_motor_driver(void)
{
	uint8_t led_value = 0;

	test_motor_driver_init();

	for(;;) {

		if(timer_current_minorslot == TIMER_MINORSLOT_0) {
			timer_current_minorslot = TIMER_MINORSLOT_NONE;

			PORTA = 0xFF;

			l6205_update_pwm();

			PORTA = 0;
		}

		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			tickcnt++;
			if(tickcnt >= TICKS_TO_EVENT) {



				if(event == 0) {
					event++;

					l6205_set_speed(MOTOR_1,  -255);
					l6205_set_speed(MOTOR_2,  -255);

				} else if(event == 1) {
					event++;

					l6205_set_speed(MOTOR_1, 0);
					l6205_set_speed(MOTOR_2, 0);

				} else if(event == 2) {
					event++;

					l6205_set_speed(MOTOR_1,  255);
					l6205_set_speed(MOTOR_2,  255);

				}else if(event == 3) {
					event = 0;

					l6205_set_speed(MOTOR_1, 0);
					l6205_set_speed(MOTOR_2, 0);
				}
				tickcnt = 0;
			}

		}
	}
}

void test_motor_driver_init(void)
{
	timer_init();
	l6205_init(L6205_ACCELERATION_DEFAULT);

	UART_init();

	DDRA = 0xFF;
	tickcnt = 0;
	event = 0;

	sei();

	printf("inited...\n");
}
