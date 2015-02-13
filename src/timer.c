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

#define SYSTEMTICKS_MAX			4

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */
volatile timer_slot_t timer_current_majorslot;
volatile timer_slot_t timer_current_minorslot;


/* * local objects               * */
static volatile uint8_t timer_systemticks;

/* * local function declarations * */
void timer_init_systemtick(void);
void timer_init_pwm(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
void timer_init_systemtick(void)
{
	timer_current_majorslot = TIMER_MAJORSLOT_NONE;
	timer_current_minorslot = TIMER_MINORSLOT_NONE;

	timer_systemticks = 0;


	// IRQ Timer with Timer0 as "system ticker" (Generate interrupt every 1 ms)
	TCCR0  = _BV(WGM01);				// Mode Select: CTC (Clear Timer on Compare)

	TCCR0 |= _BV(CS02);					// Clock Select: clk/64

	TIMSK |= _BV(OCIE0);				// Enable CTC Match Interrupt

	OCR0  = 249;						// Set Output Compare Register value

}

void timer_init_pwm(void)
{
	// Phase Correct PWM with Timer1 on PWM3 PB6/OC1B) and PWM4 (PB7/OC1C)

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
	if(timer_systemticks == 1) {
		timer_current_majorslot = TIMER_MAJORSLOT_0;
		timer_systemticks++;
	} else if (timer_systemticks >  SYSTEMTICKS_MAX) {
		timer_current_majorslot = TIMER_MAJORSLOT_1;

		timer_systemticks = 0;
	}

	timer_systemticks++;

	timer_current_minorslot = TIMER_MINORSLOT_0;



	//timer_twi_ready_timeout--;
}
