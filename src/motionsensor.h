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

#define MOTIONSENSOR_ACCELERATION_1G				16384
#define MOTIONSENSOR_COMPLEMTARY_FILTER_RATIO_BASE	1000

/* * external type and constants * */
typedef int16_t motionsensor_acceleration_t;
typedef int16_t motionsensor_angularvelocity_t;
typedef int16_t motionsensor_temperature_t;
typedef int16_t motionsensor_angle_t;


typedef struct {
	motionsensor_acceleration_t x;
	motionsensor_acceleration_t y;
	motionsensor_acceleration_t z;
} acceleration_vector_t;

typedef struct {
	motionsensor_angularvelocity_t x;
	motionsensor_angularvelocity_t y;
	motionsensor_angularvelocity_t z;
} angularvelocity_vector_t;

typedef struct {
	acceleration_vector_t acceleration;
	angularvelocity_vector_t angularvelocity;
	motionsensor_temperature_t temperature;
} motionsensor_motiondata_t;

/* * external objects            * */

//TODO: Änderungen am Modul:
// - Temperaturabhäniger Offset für angularvelocity
// - printdata modul -> nach sprintf umbauen; und etwas geschicker aufbauen.
// - qulität des betrags des bechleunigungsvektors definieren

/**
 * Normierung:
 * Sollwert == 0°
 * Min: -pi/2 == -90 deg == -1,5707963267949 rad ~= -1,571
 * Max: +pi/2 == +90 deg == -1,5707963267949 rad ~= +1,571
 *
 * => Magic numbers für 2^13
 *   - rad to int convertion: (2^13/pi/2) RAD2INT13   == 5215,189175235 => 5215
 *   - integrated_gyro to int:
 * @return
 */
extern motionsensor_angle_t motionsensor_get_angle_y(void);

extern void motionsensor_init(void);

extern void motionsensor_calibrate_zero_point(void);

//extern void motionsensor_acceleration_calibrate_zero_point(void);
//extern void motionsensor_angularvelocity_calibrate_zero_point(void);

//Getter and Setters
extern void motionsensor_get_acceleration_offset_vector(acceleration_vector_t *acceleration);
extern void motionsensor_set_acceleration_offset_vector(acceleration_vector_t *acceleration);

extern void motionsensor_get_angularvelocity_offset_vector(angularvelocity_vector_t *angularvelocity);
extern void motionsensor_set_angularvelocity_offset_vector(angularvelocity_vector_t *angularvelocity);

extern uint16_t motionsensor_get_complementary_filter_ratio(void);
extern void motionsensor_set_complementary_filter_ratio(uint16_t ratio);

extern uint16_t motionsensor_get_angle_scalingfactor(void);
extern void motionsensor_set_angle_scalingfactor(uint16_t s);

extern void motionsensor_get_raw_motiondata(motionsensor_motiondata_t * mdata);
extern void motionsensor_get_filtered_motiondata(motionsensor_motiondata_t * mdata);

extern motionsensor_angle_t motionsensor_get_angle_acceleration(void);
extern double motionsensor_get_angle_acceleration_magnitude(void);

/*
extern double motionsensor_get_complementary_filter_angularvelocity_factor(void);
extern void motionsensor_set_complementary_filter_angularvelocity_factor(double factor);

extern double motionsensor_get_complementary_filter_acceleraton_factor(void);
extern void motionsensor_set_complementary_filter_acceleraton_factor(double factor);
*/


/* * external functions          * */

#endif /* MOTIONSENSOR_H_ */
