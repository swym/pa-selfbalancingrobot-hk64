/*
 * l6205pd.c
 *
 *  Created on: Feb 12, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "l6205pd.h"

/* * system headers              * */
#include <avr/interrupt.h>
#include <avr/iom64.h>
#include <stdlib.h>

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

// PORT declaration
#define DDR_MCTRL		DDRC
#define PORT_MCTRL		PORTC

#define	DDR_MOTOR_PWM	DDRB
#define	PORT_MOTOR_PWM	PORTB

// PIN declarations
#define MCTRL_M11		PC4
#define MCTRL_M12		PC5
#define MCTRL_M21		PC6
#define MCTRL_M22		PC7

#define PWM_M1			PB6
#define PWM_M2			PB7

#define MOTOR_DEFAULT_RAMP_ACCEL	20

#define MOTOR_SPEED_MIN				(-255)
#define MOTOR_SPEED_MAX				  255

/* * local objects               * */
static motor_t motor_1;
static motor_t motor_2;

/* * local function declarations * */
void motor_update_motor_pwm(motor_t * m);


/* *** FUNCTION DEFINITIONS ************************************************* */

/*
void init_pwm_timer(void)
{
	// Phase Correct PWM with Timer1 on PWM3 PB6/OC1B) and PWM4 (PB7/OC1C)

	TCCR1A |= _BV(WGM10);				// Mode Select: 1 - PWM, Phase Correct; TOP is 0x00FF

	TCCR1A |= _BV(COM1B1);				// Compare Output Mode for PWM3: Set on upcountung
	TCCR1A |= _BV(COM1C1);				// Compare Output Mode for PWM4: Set on upcountung

	TCCR1B |= _BV(CS10);				// Clock Select: clk/1

	OCR1B   = 128;						// initial compare value
	OCR1C	= 128;
}

void init_irq_timer(void)
{
	// IRQ Timer with Timer2 as "system ticker" (~16 ms)

	TCCR2  = _BV(WGM21);				// Mode Select: CTC (Clear Timer on Compare)

	TCCR2 |= _BV(CS21);					// Clock Select: clk/8

	TIMSK |= _BV(OCIE2);				// Enable CTC Match Interrupt

	OCR2  = 249;						// Set Output Compare Register value
}
*/

void motor_set_speed(motor_id_t motor, int16_t new_speed)
{
	motor_t * m;
	m = NULL;

	if(motor == MOTOR_1) {
		m = &motor_1;
	} else if(motor == MOTOR_2) {
		m = &motor_2;
	}

	if(m != NULL) {

		if( new_speed > MOTOR_SPEED_MAX) {
			new_speed = MOTOR_SPEED_MAX;
		}
		if( new_speed < MOTOR_SPEED_MIN) {
			new_speed = MOTOR_SPEED_MIN;
		}

		m->speed_setpoint = new_speed;
	}
}

void motor_update_pwm(void)
{
	motor_update_motor_pwm(&motor_1);
	motor_update_motor_pwm(&motor_2);
}

void motor_update_motor_pwm(motor_t * m)
{
	uint16_t speed_diff;
	uint8_t  ramp_step;
	uint8_t  mctrl_tmp;

	if(m != NULL) {

		//  determine actual ramp step value
		//calculate difference between setpoint and current
		speed_diff = abs(m->speed_setpoint - m->speed_current);

		//decide size of step
		if(speed_diff > m->ramp_acceleration) {
			//prepare a normal ramp step
			ramp_step = m->ramp_acceleration;
		} else {
			//prepare the last step to reach speed_setpoint
			ramp_step = speed_diff;
		}

		//calculate one ramp step and refresh speed_current
		if(m->speed_setpoint > m->speed_current) {		// positive acceleration
			m->speed_current += ramp_step;
		} else {										// negative acceleration
			m->speed_current -= ramp_step;
		}

		//decide turning direction:
		//save old mctrl state, delete and set valid control bits of current motor
		mctrl_tmp  = PORT_MCTRL;
		mctrl_tmp &= ~(_BV(m->mctrl_in1) | _BV(m->mctrl_in2));
		if(m->speed_current >= 0) {
			mctrl_tmp |= _BV(m->mctrl_in1);
		} else {
			mctrl_tmp |= _BV(m->mctrl_in2);
		}
		PORT_MCTRL = mctrl_tmp;

		//write new pwm duty to motor driver
		if( m->speed_current > 0) {
			*m->pwm_ocr_ptr =  m->speed_current;
		} else {
			*m->pwm_ocr_ptr = -m->speed_current;
		}
	}
}

void init_motors(void)
{
	// set Motor Ctrl Pins as output
	DDR_MCTRL = _BV(MCTRL_M11) | _BV(MCTRL_M12) | _BV(MCTRL_M21) | _BV(MCTRL_M22);

	// set M1 (PWM3) and M2 (PWM4) as output
	DDR_MOTOR_PWM = _BV(PWM_M1) | _BV(PWM_M2);

	motor_1.speed_current = 0;
	motor_1.speed_setpoint = 0;
	motor_1.ramp_acceleration = MOTOR_DEFAULT_RAMP_ACCEL;
	motor_1.mode = 0;
	motor_1.mctrl_in1 = MCTRL_M11;
	motor_1.mctrl_in2 = MCTRL_M12;
	motor_1.pwm_ocr_ptr = &OCR1B;

	motor_2.speed_current = 0;
	motor_2.speed_setpoint = 0;
	motor_2.ramp_acceleration = MOTOR_DEFAULT_RAMP_ACCEL;
	motor_2.mode = 0;
	motor_2.mctrl_in1 = MCTRL_M21;
	motor_2.mctrl_in2 = MCTRL_M22;
	motor_2.pwm_ocr_ptr = &OCR1C;
}
