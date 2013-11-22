/*
 * controller.c
 *
 *  Created on: Nov 20, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "controller.h"

#include <avr/interrupt.h>
#include <stdbool.h>

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
volatile bool timer0_compare_reached = false;
/* local function declarations  */

/*
				Timer0 wird auf eine Periodenlänge von T = 4 ms einstellt

				F_OC0 = 250

				OCR0    = ?

				F_OC0   =    (F_CPU) / (2 * Prescaler * (1 + OCR0))
				OCR0    =   ((F_CPU) / (2 * Prescaler * F_OC0)) - 1
				OCR0    =  (16000000 / (2 * 256 * 250)) - 1
				OCR0    =  124

	Vgl.:	Atmel-2490-8-bit-AVR-Microcontroller-ATmega64-L_datasheet.pdf
			Seite 125
*/

/* *** FUNCTION DEFINITIONS ************************************************** */
void controller_init(void)
{
	/* Timer0 wird auf eine Periodenlänge von T = 4 ms einstellt */

	//Timer0 Betriebsart festlegen - Timer/Counter Control Register:
	TCCR0 |= (1 << WGM01); 				// Mode 4 - CTC

	//Timer1 Prescaler festlegen
	TCCR0 |= (1 << CS02 | 1 << CS01);	//Prescaler = 256

	//Interruptverhalten definieren
	TIMSK |= (1 << OCIE0);				//OCIE1A: Timer/Counter0,
										//Output Compare Match Interrupt Enable

	//Timer0 OCR-Register vorladen
	OCR0 = 124;						//OCR0: Output Compare Register
}

void controller_run(void)
{
	uint8_t led_value = 0;

	for(;;) {

		if(timer0_compare_reached) {

			led_value++;

			timer0_compare_reached = false;
		}
		PORTC = led_value;
	}
}

ISR(TIMER0_COMP_vect)
{
	timer0_compare_reached = true;
}
