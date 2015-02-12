/*
 * l6205pd.h
 *
 *  Created on: Feb 12, 2015
 *      Author: alexandermertens
 */

#ifndef L6205PD_H_
#define L6205PD_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
typedef struct {
	int16_t speed_current;
	int16_t speed_setpoint;
	uint8_t ramp_acceleration;
	uint8_t mode;
	uint8_t mctrl_in1;
	uint8_t mctrl_in2;
	volatile uint16_t * pwm_ocr_ptr;
} motor_t;

typedef uint8_t motor_id_t;

/* * external objects            * */
#define MOTOR_1		0
#define MOTOR_2		1

/* * external functions          * */
extern void init_motors(void);
extern void motor_set_speed(motor_id_t motor, int16_t new_speed);

#endif /* L6205PD_H_ */
