/*
 * timer.c
 *
 *  Created on: 25.11.2013
 *      Author: alexandermertens
 */


#include "timer.h"

#include <avr/interrupt.h>




void timer_init()
{
	timer_compare_reached = false;
	timer_slot_cnt = 0;


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


ISR(TIMER0_COMP_vect)
{
	if(timer_slot_cnt >= TIMER_SLOT_COUNTER_MAX) {
		timer_slot_cnt = 0;
	} else {
		timer_slot_cnt++;
	}
	timer_compare_reached = true;
}
