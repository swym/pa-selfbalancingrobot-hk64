/*
 * motionsensor.h
 *
 *  Created on: Jun 28, 2014
 *      Author: alexandermertens
 */


#ifndef MOTIONSENSOR_H_
#define MOTIONSENSOR_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} angularvelocity_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} acceleration_t;

typedef struct {
	angularvelocity_t angularvelocity;
	acceleration_t acceleration;
} motion_t;

/* * external objects            * */
extern void motionsensor_init(void);
extern void motionsensor_set_zero_point(void);

/**
 * Normierung:
 * Sollwert == 0°
 * Min: -pi/2 == -90 deg == -1,5707963267949 rad == -16384 int == -2^14
 * Max: +pi/2 == +90 deg == +1,5707963267949 rad == +16384 int == +2^14
 *
 * => Magic numbers:
 *   - rad to int convertion:  RAD2INT14   == 10430,37835047045352 => 10430
 *   - integrated_gyro to int: AVELO2INT14 == 1,425 (Nachverfahren: 1,57 rad * r_index<<1: pi/2 = 11500)
 * @return
 */
extern int16_t motionsensor_get_position(void);

//TODO: make static when working
extern void motionsensor_get_current_angularvelocity(angularvelocity_t *angularvelocity);
extern void motionsensor_get_current_acceleration(acceleration_t *acceleration);
//Getter and Setters
extern void motionsensor_get_acceleration_offset(acceleration_t *acceleration);
extern void motionsensor_set_acceleration_offset(acceleration_t *acceleration);

extern void motionsensor_get_angularvelocity_offset(angularvelocity_t *angularvelocity);
extern void motionsensor_set_angularvelocity_offset(angularvelocity_t *angularvelocity);

extern double motionsensor_get_complementary_filter_angularvelocity_factor(void);
extern void motionsensor_set_complementary_filter_angularvelocity_factor(double factor);

extern double motionsensor_get_complementary_filter_acceleraton_factor(void);
extern void motionsensor_set_complementary_filter_acceleraton_factor(double factor);

extern uint16_t motionsensor_get_position_multiplier(void);
extern void motionsensor_set_position_multiplier(uint16_t position_multiplier);

/* * external functions          * */

#endif /* MOTIONSENSOR_H_ */
