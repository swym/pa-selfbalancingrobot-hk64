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
typedef int16_t angularvelocity_t_;
typedef int16_t acceleration_t_;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} acceleration_vector_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} angularvelocity_vector_t;
/*
typedef struct {
	acceleration_vector_t acceleration;
	angularvelocity_vector_t angularvelocity;
} motion_t_edit;

typedef struct {
	acceleration_vector_t acceleration;
	angularvelocity_vector_t angularvelocity;
} motionsensor_motiondata;
*/

/* * external objects            * */

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
extern int16_t motionsensor_get_angle_y(void);

extern void motionsensor_init(void);

extern void motionsensor_acceleration_calibrate_zero_point(void);
extern void motionsensor_angularvelocity_calibrate_zero_point(void);

//Getter and Setters
extern void motionsensor_get_acceleration_offset(acceleration_vector_t *acceleration);
extern void motionsensor_set_acceleration_offset(acceleration_vector_t *acceleration);

extern void motionsensor_get_angularvelocity_offset(angularvelocity_vector_t *angularvelocity);
extern void motionsensor_set_angularvelocity_offset(angularvelocity_vector_t *angularvelocity);

extern double motionsensor_get_complementary_filter_angularvelocity_factor(void);
extern void motionsensor_set_complementary_filter_angularvelocity_factor(double factor);

extern double motionsensor_get_complementary_filter_acceleraton_factor(void);
extern void motionsensor_set_complementary_filter_acceleraton_factor(double factor);

extern uint16_t motionsensor_get_angle_y_scalingfactor(void);
extern void motionsensor_set_angle_y_scalingfactor(uint16_t s);

/* * external functions          * */

#endif /* MOTIONSENSOR_H_ */
