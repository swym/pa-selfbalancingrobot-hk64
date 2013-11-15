/*
 * md25.h
 *
 *  Created on: Nov 15, 2013
 *      Author: alexandermertens
 */


#ifndef MD25_H_
#define MD25_H_

/* *** INCLUDES ************************************************************** */

/* system headers              */ 
#include <stdbool.h>
#include <stdint.h>

/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */
#define MD25_MOTOR1					0
#define MD25_MOTOR2					1

#define MD25_MODE_UNSIGNED_SPEED	0
#define MD25_MODE_SIGNED_SPEED		1
#define MD25_MODE_UNSIGNED_TURN		2
#define MD25_MODE_SIGNED_TURN		3

#define MD25_ACCELERATION_MIN		1
#define MD25_ACCELERATION_MAX		10

/* external objects            */

/* external functions          */
extern void md25_set_speed(int8_t, int8_t); /*int8 motor1, int8 motor2 */

extern void md25_set_motor1_speed(int8_t);   /* int8_t speed */
extern void md25_set_motor2_speed(int8_t);   /* int8_t speed */

extern int8_t md25_get_motor1_speed(void);
extern int8_t md25_get_motor2_speed(void);

extern void md25_set_acceleration_rate(uint8_t); /* uint8_t accerleration */
extern uint8_t md25_get_acceleration_rate(void);

extern void md25_set_mode(uint8_t); /*uint8_t mode */
extern uint8_t md25_get_mode(void);

extern uint32_t md25_get_motor_encoder(uint8_t); /* uint8_t motor */
extern uint8_t md25_get_battery_volts(void);
extern uint8_t md25_get_motor_current(uint8_t); /* uint8_t motor */
extern uint8_t md25_get_software_revision(void);

extern void md25_cmd_reset_encoders(void);
extern void md25_cmd_enable_speed_regulation(bool); /*bool enable */
extern void md25_cmd_enable_i2c_timout(bool);		/*bool enable */
//TODO: Implement functions for changing the i2c bus address

#endif /* MD25_H_ */



