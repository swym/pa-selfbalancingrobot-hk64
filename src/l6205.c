/*
 * l6205pd.c
 *
 *  Created on: Feb 12, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include <avr/interrupt.h>
#include <avr/iom64.h>
#include <stdlib.h>
#include "l6205.h"

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

// PORT declaration
#define DDR_MOTORCTRL	DDRC
#define PORT_MOTORCTRL	PORTC

#define	DDR_PWM			DDRB
#define	PORT_PWM		PORTB

// PIN declarations
#define MOTORCTRL_M11	PC4
#define MOTORCTRL_M12	PC5
#define MOTORCTRL_M21	PC6
#define MOTORCTRL_M22	PC7

#define PWM3			PB6
#define PWM4			PB7




/* * local objects               * */
static motor_t motor_1;
static motor_t motor_2;

/* * local function declarations * */
void l6205_update_pwm_of_motor(motor_t * m);


/* *** FUNCTION DEFINITIONS ************************************************* */

void l6205_init(uint8_t acceleration)
{
	// set Motor Ctrl Pins as output
	DDR_MOTORCTRL = _BV(MOTORCTRL_M11) |
			        _BV(MOTORCTRL_M12) |
					_BV(MOTORCTRL_M21) |
					_BV(MOTORCTRL_M22);

	// set M1 (PWM3) and M2 (PWM4) as output
	DDR_PWM = _BV(PWM3) | _BV(PWM4);


	if(acceleration > L6205_ACCELERATION_MAX) {
		acceleration = L6205_ACCELERATION_MAX;
	}

	if(acceleration == 0) {
		acceleration = L6205_ACCELERATION_DEFAULT;
	}

	motor_1.speed_current  = 0;
	motor_1.speed_setpoint = 0;
	motor_1.acceleration   = acceleration;
	motor_1.mctrl_in1      = MOTORCTRL_M11;
	motor_1.mctrl_in2      = MOTORCTRL_M12;
	motor_1.pwm_ocr_ptr    = &OCR1B;

	motor_2.speed_current  = 0;
	motor_2.speed_setpoint = 0;
	motor_2.acceleration   = acceleration;
	motor_2.mctrl_in1      = MOTORCTRL_M22;   //swap in1 and in2 so speed changed result in fwd or backwd
	motor_2.mctrl_in2      = MOTORCTRL_M21;
	motor_2.pwm_ocr_ptr    = &OCR1C;
}

void l6205_set_speed(motor_id_t motor, int16_t new_speed)
{
	motor_t * m;
	m = NULL;

	if(motor == MOTOR_1) {
		m = &motor_1;
	} else if(motor == MOTOR_2) {
		m = &motor_2;
	}

	if(m != NULL) {

		if(new_speed > 0) {
			new_speed += L6205_DEADZONE_OFFSET;
		} else if(new_speed < 0) {
			new_speed -= L6205_DEADZONE_OFFSET;
		}

		if( new_speed > L6205_MOTOR_SPEED_MAX) {
			new_speed = L6205_MOTOR_SPEED_MAX;
		}
		if( new_speed < L6205_MOTOR_SPEED_MIN) {
			new_speed = L6205_MOTOR_SPEED_MIN;
		}

		m->speed_setpoint = new_speed;
	}
}

void l6205_handler(void)
{
	l6205_update_pwm_of_motor(&motor_1);
	l6205_update_pwm_of_motor(&motor_2);
}

void l6205_update_pwm_of_motor(motor_t * m)
{
	uint16_t speed_diff;
	uint8_t  step;
	uint8_t  mctrl_tmp;

	if(m != NULL) {

		//  determine actual ramp step value
		//calculate difference between setpoint and current
		speed_diff = abs(m->speed_setpoint - m->speed_current);

		//decide size of step
		if(speed_diff > m->acceleration) {
			//prepare a normal ramp step
			step = m->acceleration;
		} else {
			//prepare the last step to reach speed_setpoint
			step = speed_diff;
		}

		//calculate one ramp step and refresh speed_current
		if(m->speed_setpoint > m->speed_current) {		// positive acceleration
			m->speed_current += step;
		} else {										// negative acceleration
			m->speed_current -= step;
		}

		//decide turning direction:
		//save old mctrl state, delete and set valid control bits of current motor
		mctrl_tmp  = PORT_MOTORCTRL;
		mctrl_tmp &= ~(_BV(m->mctrl_in1) | _BV(m->mctrl_in2));
		if(m->speed_current >= 0) {
			mctrl_tmp |= _BV(m->mctrl_in1);
		} else {
			mctrl_tmp |= _BV(m->mctrl_in2);
		}
		PORT_MOTORCTRL = mctrl_tmp;

		//write new pwm duty to motor driver
		if( m->speed_current > 0) {
			*m->pwm_ocr_ptr =  m->speed_current;
		} else {
			*m->pwm_ocr_ptr = -m->speed_current;
		}
	}
}
