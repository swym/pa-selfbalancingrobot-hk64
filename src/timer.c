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
#include "l6205.h"
#include "encoder.h"
#include "motor_control.h"


/* *** DEFINES ************************************************************** */

#define SYSTEMTICKS_MAX			3
#define SYSTEMTICKS_COUNT		(SYSTEMTICKS_MAX+1) 	// Systemticker counts from 0 .. 3 == 4

/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */
volatile timer_slot_t timer_current_majorslot;

volatile uint16_t timer_command_timeout;
volatile uint8_t  timer_20ms_ticker;


/* * local objects               * */
static volatile uint8_t timer_systemticks;

/* * local function declarations * */
void timer_init_systemtick(void);
void timer_init_pwm(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
void timer_init_systemtick(void)
{
	timer_current_majorslot = TIMER_MAJORSLOT_NONE;

	timer_systemticks = 0;
	timer_20ms_ticker = 0;


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

	TCCR1A |= _BV(COM1B1);				// Compare Output Mode for PWM3: Set on up counting
	TCCR1A |= _BV(COM1C1);				// Compare Output Mode for PWM4: Set on up counting

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
	//set TIMER Flags
	if(timer_systemticks == 0) {
		timer_current_majorslot = TIMER_MAJORSLOT_0;
	} else if (timer_systemticks == 2) {
		timer_current_majorslot = TIMER_MAJORSLOT_1;
	}

	//increment systemticker or reset cycle on max
	if (timer_systemticks >= SYSTEMTICKS_MAX) {
		timer_systemticks = 0;
	} else {
		timer_systemticks++;
	}

	//decrement command_timeout
	if(timer_command_timeout > 0) {
		timer_command_timeout--;
	}

	//update external handlers
	l6205_handler();
	encoder_handler();

	//update only every 20 ms
	timer_20ms_ticker++;
	if(timer_20ms_ticker >= 20) {
		timer_20ms_ticker = 0;
		motor_control_handler();

	}
}
