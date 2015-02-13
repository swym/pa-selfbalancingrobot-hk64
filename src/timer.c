/*
 * timer.c
 *
 *  Created on: Feb 13, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "timer.h"

/* * system headers              * */
#include <avr/interrupt.h>

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
static volatile uint8_t timer_slot_counter;

/* * local function declarations * */
void timer_init_systemtick(void);
void timer_init_pwm(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
void timer_init_systemtick(void)
{

	timer_slot_0 = false;
	timer_slot_1 = false;
//	timer_slot_2 = false;
//	timer_slot_3 = false;
	timer_slot_counter = 0;


	/* Timer0 wird auf eine Periodenl�nge von T = 2 ms einstellt */

	//Timer0 Betriebsart festlegen - Timer/Counter Control Register:
	TCCR0 |= (1 << WGM01); 				// Mode 4 - CTC

	//Timer1 Prescaler festlegen
	//TCCR0 |= (1 << CS02 | 1 << CS01);	//Prescaler = 256
	TCCR0 |= (1 << CS02 | 1 << CS00);	//Prescaler = 128
	//TCCR0 |= (1 << CS02);				//Prescaler = 64

	//Interruptverhalten definieren
	TIMSK |= (1 << OCIE0);				//OCIE0: Timer/Counter0,
										//Output Compare Match Interrupt Enable

	//Timer0 OCR-Register vorladen
	OCR0 = 249;							//OCR0: Output Compare Register
	//OCR0 = 124;							//OCR0: Output Compare Register

}

void timer_init_pwm(void)
{
	TCCR1A |= _BV(WGM10);				// Mode Select: 1 - PWM, Phase Correct; TOP is 0x00FF

	TCCR1A |= _BV(COM1B1);				// Compare Output Mode for PWM3: Set on upcountung
	TCCR1A |= _BV(COM1C1);				// Compare Output Mode for PWM4: Set on upcountung

	TCCR1B |= _BV(CS10);				// Clock Select: clk/1

	OCR1B   = 0;						// initial compare value
	OCR1C	= 0;
}

void timer_init()
{
	timer_init_systemtick();
	timer_init_pwm();
}

ISR(TIMER0_COMP_vect)
{

	if(timer_slot_counter == 0) {
		timer_slot_0 = true;
	}

	if (timer_slot_counter == 1){
		timer_slot_1 = true;
	}

	if (timer_slot_counter >= 1){
		timer_slot_counter = 0;
	} else {
		timer_slot_counter++;
	}

	timer_twi_ready_timeout--;
}
