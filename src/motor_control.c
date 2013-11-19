/*
 * motor_control.c
 *
 *  Created on: Nov 19, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "motor_control.h"

#include "md25.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
static motor_contol_speed_t current_speed;
static motor_contol_speed_t new_speed;

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */
void motor_control_init()
{
	current_speed.motor_1 = 0;
	current_speed.motor_2 = 0;

	new_speed.motor_1 = 0;
	new_speed.motor_2 = 0;

	md25_init();
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
	md25_set_speed(new_speed.motor_1, new_speed.motor_2);

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
