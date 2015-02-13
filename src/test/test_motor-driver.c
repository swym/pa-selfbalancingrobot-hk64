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

			motor_update_pwm();

			PORTA = 0;
		}

		if(timer_current_majorslot == TIMER_MAJORSLOT_0) {
			timer_current_majorslot = TIMER_MAJORSLOT_NONE;

			tickcnt++;
			if(tickcnt >= TICKS_TO_EVENT) {



				if(event == 0) {
					event++;

					motor_set_speed(MOTOR_1,  255);
					motor_set_speed(MOTOR_2, -255);

				} else if(event == 1) {
					event++;

					motor_set_speed(MOTOR_1, 0);
					motor_set_speed(MOTOR_2, 0);

				} else if(event == 2) {
					event++;

					motor_set_speed(MOTOR_1, -255);
					motor_set_speed(MOTOR_2,  255);

				}else if(event == 3) {
					event = 0;

					motor_set_speed(MOTOR_1, 0);
					motor_set_speed(MOTOR_2, 0);
				}
				tickcnt = 0;
			}

		}
	}
}

void test_motor_driver_init(void)
{
	timer_init();
	init_motors();

	UART_init(57600);

	DDRA = 0xFF;
	tickcnt = 0;
	event = 0;

	sei();

	printf("inited...\n");
}
