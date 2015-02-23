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

//TODO: Änderungen am Modul:
// - Filter wieder einbinden.
// - Filter vereinheitlichen
// - Temperaturabhäniger Offset für angularvelocity
// - printdata modul -> nach sprintf umbauen; und etwas geschicker aufbauen.

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

extern void motionsensor_calibrate_zero_point(void);

//extern void motionsensor_acceleration_calibrate_zero_point(void);
//extern void motionsensor_angularvelocity_calibrate_zero_point(void);

extern void motionsensor_printdata(void);

//Getter and Setters
extern void motionsensor_get_acceleration_offset_vector(acceleration_vector_t *acceleration);
extern void motionsensor_set_acceleration_offset_vector(acceleration_vector_t *acceleration);

extern void motionsensor_get_angularvelocity_offset_vector(angularvelocity_vector_t *angularvelocity);
extern void motionsensor_set_angularvelocity_offset_vector(angularvelocity_vector_t *angularvelocity);

extern uint8_t motionsensor_get_complementary_filter_ratio(void);
extern void motionsensor_set_complementary_filter_ratio(uint8_t ratio);

extern uint16_t motionsensor_get_angle_scalingfactor(void);
extern void motionsensor_set_angle_scalingfactor(uint16_t s);

/*
extern double motionsensor_get_complementary_filter_angularvelocity_factor(void);
extern void motionsensor_set_complementary_filter_angularvelocity_factor(double factor);

extern double motionsensor_get_complementary_filter_acceleraton_factor(void);
extern void motionsensor_set_complementary_filter_acceleraton_factor(double factor);
*/


/* * external functions          * */

#endif /* MOTIONSENSOR_H_ */
