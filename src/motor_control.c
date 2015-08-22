/*
 * motor_control.c
 *
 *  Created on: Nov 19, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "motor_control.h"

#include "l6205.h"
#include "encoder.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */
#define WHEEL_POS_MAX 0x7ffff000L
#define WHEEL_POS_MIN (-WHEEL_POS_MAX - 1L)

#define ROBOT_POS_MAX 0x7fffff00L
#define ROBOT_POS_MIN (-ROBOT_POS_MAX - 1L)

#define ROBOT_POS_HALTZONE	610

#define MOTOR_RUNAWAY_MAX_COUNT 750
#define MOTOR_RUNAWAY_MAX		(2 * L6205_MOTOR_SPEED_MAX)
#define MOTOR_RUNAWAY_MIN		(2 * L6205_MOTOR_SPEED_MIN)

static motor_control_speed_t current_speed;
static motor_control_speed_t new_speed;

static motor_control_position_t wheel_position;
static int32_t robot_position;
static int32_t real_robot_position;

static uint16_t motor_runaway_counter;


/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */

/**
 * Initialize the Motor-Controller and the underlaying Motor-driver MD25
 */
void motor_control_init(uint8_t motor_acceleration)
{
	current_speed.motor_1 = 0;
	current_speed.motor_2 = 0;

	new_speed.motor_1 = 0;
	new_speed.motor_2 = 0;

	motor_control_reset_position();

	encoder_init();
	l6205_init(motor_acceleration);
}


/**
 * Saves the new calculated motor_speeds. To set the new speeds a call of
 * motor_control_set_new_speed() must be performed.
 * @param s
 */
void motor_control_prepare_new_speed(motor_control_speed_t *s)
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
}

/**
 *Returns the current_speed struct
 * @param s
 */
void motor_control_get_current_speed(motor_control_speed_t *s)
{
	s->motor_1 = current_speed.motor_1;
	s->motor_2 = current_speed.motor_2;
}

/**
 * Returns the new_speed sturct
 * @param s
 */
void motor_control_get_new_speed(motor_control_speed_t *s)
{
	s->motor_1 = new_speed.motor_1;
	s->motor_2 = new_speed.motor_2;
}

int16_t motor_control_get_robot_speed(void)
{
	return (current_speed.motor_1 + current_speed.motor_2);
}

int32_t motor_control_get_robot_position(void)
{
	return robot_position;
}

int32_t motor_control_get_real_robot_position(void)
{
	return real_robot_position;
}

bool motor_control_motors_runaway(void)
{
	bool retval = false;

	if((new_speed.motor_1 + new_speed.motor_2) >= MOTOR_RUNAWAY_MAX ||
	   (new_speed.motor_1 + new_speed.motor_2) <= MOTOR_RUNAWAY_MIN) {

		motor_runaway_counter++;

		if(motor_runaway_counter > MOTOR_RUNAWAY_MAX_COUNT) {
			retval = true;
		}

	} else {
		motor_runaway_counter = 0;
	}

	return retval;
}

void motor_control_reset_position(void)
{
	wheel_position.wheel_1 = 0;
	wheel_position.wheel_2 = 0;

	real_robot_position = 0;
	robot_position = 0;
}

void motor_control_handler(void)
{
	current_speed.motor_1 = -encoder_read_delta(ENCODER_M1); //invert encoder signal, because m2 turns invers to m1
	current_speed.motor_2 =  encoder_read_delta(ENCODER_M2);

	if(wheel_position.wheel_1 > WHEEL_POS_MIN && wheel_position.wheel_1 < WHEEL_POS_MAX) {
		wheel_position.wheel_1 += current_speed.motor_1;
	}

	if(wheel_position.wheel_2 > WHEEL_POS_MIN && wheel_position.wheel_2 < WHEEL_POS_MAX) {
		wheel_position.wheel_2 += current_speed.motor_2;
	}

	if(real_robot_position > ROBOT_POS_MIN && real_robot_position < ROBOT_POS_MAX) {
		real_robot_position = (wheel_position.wheel_1 + wheel_position.wheel_2) / 2;
	}

	if(real_robot_position > ROBOT_POS_HALTZONE) {
		robot_position = real_robot_position - ROBOT_POS_HALTZONE;
	} else if(real_robot_position < -ROBOT_POS_HALTZONE) {
		robot_position = real_robot_position + ROBOT_POS_HALTZONE;
	} else {
		robot_position = 0;
	}
}
