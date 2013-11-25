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



#include <stdbool.h>
#include <math.h>
#include <avr/io.h>

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */


#define LED1 1
#define LED2 2
#define LED3 4


/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */

/*
				Timer0 wird auf eine Periodenl�nge von T = 4 ms einstellt

				F_OC0 = 250

				OCR0    = ?

				F_OC0   =    (F_CPU) / (2 * Prescaler * (1 + OCR0))
				OCR0    =   ((F_CPU) / (2 * Prescaler * F_OC0)) - 1
				OCR0    =  (16000000 / (2 * 256 * 250)) - 1
				OCR0    =  124

	Vgl.:	Atmel-2490-8-bit-AVR-Microcontroller-ATmega64-L_datasheet.pdf
			Seite 125
*/
void controller_init(void)
{

}

void controller_run(void)
{
	uint8_t led_value = 0;
	acceleration_t current_accel;
	motor_contol_speed_t new_speed;

	double x, z, position;

	for(;;) {




		/* Bei Interrupt Slot-Counter herh�hen */
//		if(timer0_compare_reached) {
//
//			timer0_compare_reached = false;
//		}

		if(timer_compare_reached) {

			timer_compare_reached = false;
			//led_value ^= LED3;

			/* Erster Zeitslot am Anfang des Intervalls; t = 0 ms */
			if(timer_slot_cnt == 0) {
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

				position = -atan2(x, z);

				position = position * 2;

				/* TODO: als IST-Wert in den PID-Regler geben
				 * Stellgr��e an Motorsteuerung weitergeben, ABER noch nicht setzen
				 */


				new_speed.motor_1 = 10;
				new_speed.motor_2 = 10;

				motor_control_prepare_new_speed(&new_speed);
				PORTC ^= LED2;
			}

			/* Zweiter Zeitslot des Intervalls; t = 12 ms */
			if(timer_slot_cnt == 3) {
				PORTC ^= LED2;
				/*
				 * Neue Stellgr��e des Motors setzen
				 */
				motor_control_set_new_speed();
				PORTC ^= LED2;
			}

			if(timer_slot_cnt >= TIMER_SLOT_COUNTER_MAX) {
				led_value &= ~(LED1 | LED2);	//Wieder beide LEDs l�schen
			}
		}
	}
}
