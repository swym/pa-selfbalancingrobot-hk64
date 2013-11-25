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

#include <avr/interrupt.h>
#include <stdbool.h>
#include <math.h>

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
#define SLOT_COUNTER_MAX 4 // (Intervalll�nge = 20 ms / IRQ-Zeit = 4 ms)-1 -> 5-1

#define LED1 1
#define LED2 2
#define LED3 4

volatile bool timer0_compare_reached = false;
volatile uint8_t slot_cnt = 0;

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
	/* Timer0 wird auf eine Periodenl�nge von T = 4 ms einstellt */

	//Timer0 Betriebsart festlegen - Timer/Counter Control Register:
	TCCR0 |= (1 << WGM01); 				// Mode 4 - CTC

	//Timer1 Prescaler festlegen
	TCCR0 |= (1 << CS02 | 1 << CS01);	//Prescaler = 256

	//Interruptverhalten definieren
	TIMSK |= (1 << OCIE0);				//OCIE1A: Timer/Counter0,
										//Output Compare Match Interrupt Enable

	//Timer0 OCR-Register vorladen
	OCR0 = 149;						//OCR0: Output Compare Register
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

		if(timer0_compare_reached) {

			timer0_compare_reached = false;
			//led_value ^= LED3;

			/* Erster Zeitslot am Anfang des Intervalls; t = 0 ms */
			if(slot_cnt == 0) {
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
			if(slot_cnt == 3) {
				PORTC ^= LED2;
				/*
				 * Neue Stellgr��e des Motors setzen
				 */
				motor_control_set_new_speed();
				PORTC ^= LED2;
			}

			if(slot_cnt >= SLOT_COUNTER_MAX) {
				led_value &= ~(LED1 | LED2);	//Wieder beide LEDs l�schen
			}
		}
	}
}

ISR(TIMER0_COMP_vect)
{
	if(slot_cnt >= SLOT_COUNTER_MAX) {
		slot_cnt = 0;
		PORTC ^= LED3;
	} else {
		slot_cnt++;
	}
	timer0_compare_reached = true;
}
