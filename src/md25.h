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

/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */

/* external objects            */

/* external functions          */
extern void md25_set_speed(uint8_t, uint8_t); /*uint8 motor1, uint8 motor2 */

extern void md25_set_motor1_speed(uint8_t);   /* uint8_t speed */
extern void md25_set_motor2_speed(uint8_t);   /* uint8_t speed */

extern uint8_t md25_get_motor2_speed(void);
extern uint8_t md25_get_motor2_speed(void);

extern void md25_set_acceleration_rate();
extern uint8_t md25_get_acceleration_rate(void);

extern void md25_set_mode(uint8_t);
extern uint8_t md25_get_mode(void);

extern uint32_t md25_get_motor_encoder_1(void);
extern uint32_t md25_get_motor_encoder_2(void);

extern uint8_t md25_get_battery_volts(void);
extern uint8_t md25_get_motor1_current(void);
extern uint8_t md25_get_motor2_current(void);
extern uint8_t md25_get_software_revision(void);

extern void md25_cmd_reset_encoders(void);
extern void md25_cmd_enable_speed_regulation(bool); /*bool enable */
extern void md25_cmd_enable_i2c_timout(bool);		/*bool enable */
//TODO: Implement functions for changing the i2c bus address

#endif /* MD25_H_ */



