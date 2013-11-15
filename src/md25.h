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

/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */

/* external objects            */

/* external functions          */
extern void md25_set_speed(uint8, uint8); /*uint8 motor1, uint8 motor2 */
extern void md25_set_motor1_speed();
extern void md25_set_motor2_speed();

extern void md25_get_motor2_speed();
extern void md25_get_motor2_speed();

extern void md25_set_acceleration_rate();
extern void md25_get_acceleration_rate();

extern void md25_set_mode();
extern void md25_get_mode();

extern void md25_get_motor_encoder_1();
extern void md25_get_motor_encoder_2();

extern void md25_get_battery_volts();
extern void md25_get_motor1_current();
extern void md25_get_motor2_current();
extern void md25_get_software_revision();

extern void md25_cmd_reset_encoders();
extern void md25_cmd_enable_speed_regulation(bool);
extern void md25_cmd_enable_i2c_timout(bool);
//TODO: Implement functions for changing the i2c bus address

#endif /* MD25_H_ */



