/*
 * controller.c
 *
 *  Created on: Nov 20, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "controller.h"

#include "acceleration_t.h"
#include "motor_control.h"
#include "timer.h"
#include "pid.h"



#include <stdbool.h>
#include <math.h>
#include <avr/io.h>

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */


#define LED1 1
#define LED2 2
#define LED3 4


//global Data
volatile bool timer_compare_reached;
volatile uint8_t timer_slot_counter;

/* local function declarations  */
pidData_t pid_controller_settings;



/* *** FUNCTION DEFINITIONS ************************************************** */


void controller_init(void)
{
	pid_Init(10486, 0, 0, &pid_controller_settings);
}

void controller_run(void)
{
	uint8_t led_value = 0;
	uint16_t speed = 0;
	acceleration_t current_accel;
	motor_contol_speed_t new_speed;

	double x, z, position;

	for(;;) {


		if(timer_compare_reached) {

			timer_compare_reached = false;
			//led_value ^= LED3;

			/* Erster Zeitslot am Anfang des Intervalls; t = 0 ms */
			if(timer_slot_counter == 0) {
				/*
				 * Beschleunigungswerte lesen
				 * Beschleunigungswerte in Position umrechnen
				 * als IST-Wert in den PID-Regler geben
				 * Stellgr��e an Motorsteuerung weitergeben, ABER noch nicht setzen
				 */
				PORTC ^= (LED1 | LED2);				//LED1 an.

				//Beschleunigungswerte lesen
				acceleration_get_current_acceleration(&current_accel);

				//Beschleunigungswerte in Position umrechnen
				x = (double)(current_accel.x);
				z = (double)(current_accel.z);

				position = atan2(x, z) * 100;

				/* TODO: als IST-Wert in den PID-Regler geben
				 * Stellgr��e an Motorsteuerung weitergeben, ABER noch nicht setzen
				 */


				speed = pid_Controller(0, (int16_t)(position), &pid_controller_settings);

				new_speed.motor_1 = speed >> 8;
				new_speed.motor_2 = speed >> 8;

				motor_control_prepare_new_speed(&new_speed);
				PORTC ^= LED2;
			}

			/* Zweiter Zeitslot des Intervalls; t = 12 ms */
			if(timer_slot_counter == 3) {
				PORTC ^= LED2;
				/*
				 * Neue Stellgr��e des Motors setzen
				 */
				motor_control_set_new_speed();
				PORTC ^= LED2;
			}

			if(timer_slot_counter >= TIMER_SLOT_COUNTER_MAX) {
				led_value &= ~(LED1 | LED2);	//Wieder beide LEDs l�schen
			}
		}
	}
}
