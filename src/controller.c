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
volatile bool timer1_compare_reached = false;
/* local function declarations  */

/*
Discription: Timer1 alle 10 Millisekunden einen Interrupt auslösen. OCR1A und
             der Prescaler werden dabei so gewählt, dass mit dem gegebenen Takt
             die ISR (TIMER1_COMPA_vect) entsprechend ausgelöst wird.

                F_CPU               = 16000000
                F_OC1A == t = 1/s   = 100
                Prescaler           = 64

                OCR1A = ?

				F_OC1A  =    (F_CPU) / (2 * Prescaler * (1 + OCR1A))
				OCR1A   =   ((F_CPU) / (2 * Prescaler * F_OC1A)) - 1
				OCR1A   =  (16000000 / (2 * 64 * 100)) - 1
				OCR1A   =  1249

	Vgl.:	Atmel-2490-8-bit-AVR-Microcontroller-ATmega64-L_datasheet.pdf
			Seite 125
*/

/* *** FUNCTION DEFINITIONS ************************************************** */
void controller_init(void)
{
	/* Timer1 wird auf eine Periodenlänge von T = 10 ms einstellt */

	//Timer1 Betriebsart festlegen
	TCCR1B |= (1 << WGM12); //Mode 04: CTC

	//Timer1 Prescaler festlegen
	TCCR1B |= (1 << CS11 | 1 << CS10);	//Prescaler = 64

	//Interruptverhalten definieren
	TIMSK |= (1 << OCIE1A);				//OCIE1A: Timer/Counter1,
										//Output Compare A Match Interrupt Enable

	//Timer1 OCR-Register vorladen
	OCR1A = 1249;						//OCR1A: Output Compare Register 1 A
}

void controller_run(void)
{
	uint8_t led_value = 0;

	for(;;) {

		if(timer1_compare_reached) {

			led_value++;

			timer1_compare_reached = false;
		}
		PORTC = led_value;
	}
}

ISR(TIMER1_COMPA_vect)
{
	timer1_compare_reached = true;
}
