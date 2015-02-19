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
/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */
typedef struct {
	int16_t motor_1;
	int16_t motor_2;
} motor_contol_speed_t;


/* external objects            */

/* external functions          */
extern void motor_control_init(void);

extern void motor_control_prepare_new_speed(motor_contol_speed_t *); /* motor_contol_speed_t *s */
extern void motor_control_set_new_speed(void);
//extern void motor_control_update_pwm(void);


extern void motor_control_get_current_speed(motor_contol_speed_t *); /* motor_contol_speed_t *s */
extern void motor_control_get_new_speed(motor_contol_speed_t *); /* motor_contol_speed_t *s */

#endif /* MOTOR_CONTROL_H_ */



