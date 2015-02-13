/*
 * motor_control.c
 *
 *  Created on: Nov 19, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "motor_control.h"

#include "l6205.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
static motor_contol_speed_t current_speed;
static motor_contol_speed_t new_speed;

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */

/**
 * Initialize the Motor-Controller and the underlaying Motor-driver MD25
 */
void motor_control_init()
{
	current_speed.motor_1 = 0;
	current_speed.motor_2 = 0;

	new_speed.motor_1 = 0;
	new_speed.motor_2 = 0;

	l6205_init(L6205_ACCELERATION_DEFAULT);
}


/**
 * Saves the new calculated motor_speeds. To set the new speeds a call of
 * motor_control_set_new_speed() must be performed.
 * @param s
 */
void motor_control_prepare_new_speed(motor_contol_speed_t *s)
{
	new_speed.motor_1 = s->motor_1;
	new_speed.motor_2 = s->motor_2;
}

/**
 * Sets the new Motorspeed. Calls the correspondending driver-function of the
 * md25. Also sets the struct current_speed = new_speed;
 */
void motor_control_set_new_speed()
{
	l6205_set_speed(MOTOR_1, new_speed.motor_1);
	l6205_set_speed(MOTOR_2, new_speed.motor_2);

	current_speed.motor_1 = new_speed.motor_1;
	current_speed.motor_2 = new_speed.motor_2;
}

/**
 *Returns the current_speed struct
 * @param s
 */
void motor_control_get_current_speed(motor_contol_speed_t *s)
{
	s->motor_1 = current_speed.motor_1;
	s->motor_2 = current_speed.motor_2;
}

/**
 * Returns the new_speed sturct
 * @param s
 */
void motor_control_get_new_speed(motor_contol_speed_t *s)
{
	s->motor_1 = new_speed.motor_1;
	s->motor_2 = new_speed.motor_2;
}
