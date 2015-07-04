/*
 * motor_control.h
 *
 *  Created on: Nov 19, 2013
 *      Author: alexandermertens
 */


#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

/* *** INCLUDES ************************************************************** */

/* system headers              */ 
#include <stdint.h>
#include <stdbool.h>

/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */
typedef struct {
	int16_t motor_1;
	int16_t motor_2;
} motor_control_speed_t;

typedef struct {
	int32_t wheel_1;
	int32_t wheel_2;
} motor_control_position_t;

/* external objects            */

/* external functions          */
extern void motor_control_init(uint8_t motor_acceleration);

extern void motor_control_prepare_new_speed(motor_control_speed_t *s);
extern void motor_control_set_new_speed(void);


extern void motor_control_get_current_speed(motor_control_speed_t *s);
extern void motor_control_get_new_speed(motor_control_speed_t *s);

extern int16_t motor_control_get_robot_speed(void);
extern int32_t motor_control_get_robot_position(void);
extern int32_t motor_control_get_real_robot_position(void);
extern void motor_control_reset_position(void);

extern bool motor_control_motors_runaway(void);

extern void motor_control_handler(void);

#endif /* MOTOR_CONTROL_H_ */



