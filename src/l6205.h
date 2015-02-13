/*
 * l6205pd.h
 *
 *  Created on: Feb 12, 2015
 *      Author: alexandermertens
 */

#ifndef L6205_H_
#define L6205_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
typedef struct {
	int16_t speed_current;
	int16_t speed_setpoint;
	uint8_t acceleration;
	uint8_t mctrl_in1;
	uint8_t mctrl_in2;
	volatile uint16_t * pwm_ocr_ptr;
} motor_t;

typedef uint8_t motor_id_t;

/* * external objects            * */
#define MOTOR_1		0
#define MOTOR_2		1

#define L6205_ACCELERATION_MAX		50
#define L6205_ACCELERATION_DEFAULT  20

/* * external functions          * */
extern void l6205_init(uint8_t acceleration);
extern void l6205_set_speed(motor_id_t motor, int16_t new_speed);
extern void l6205_update_pwm(void);

#endif /* L6205_H_ */
