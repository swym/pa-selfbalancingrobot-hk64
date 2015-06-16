/*
 * filters.h
 *
 *  Created on: Jul 16, 2014
 *      Author: alexandermertens
 */


#ifndef FILTER_H_
#define FILTER_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define FILTER_MOVING_GENERIC_WEIGHTS_COUNT				8
#define FILTER_MOVING_AVERAGE_ELEMENT_COUNT				8
#define FILTER_MOVING_AVERAGE_FLOAT_ELEMENT_COUNT		64

typedef struct {
	int16_t avg;
	int16_t elements[FILTER_MOVING_AVERAGE_ELEMENT_COUNT];
	int8_t  index;
	int32_t elements_sum;
} filter_moving_average_t;

typedef struct {
	float avg;
	float elements[FILTER_MOVING_AVERAGE_ELEMENT_COUNT];
	int8_t  index;
	float elements_sum;
} filter_moving_average_float_t;

typedef struct {
	int16_t avg;
	int16_t elements[FILTER_MOVING_GENERIC_WEIGHTS_COUNT];
	uint8_t weights[FILTER_MOVING_GENERIC_WEIGHTS_COUNT];
	uint8_t weights_count;
	uint8_t weights_sum;
} filter_moving_generic_average_t;

/* * external objects            * */

/* * external functions          * */
extern void filter_moving_generic_average_put_element(filter_moving_generic_average_t * average, int16_t new_value);
extern void filter_moving_generic_average_init(filter_moving_generic_average_t * average, uint8_t * weights, int16_t init_value);
extern void filter_moving_generic_average_flush(filter_moving_generic_average_t * average);

extern void filter_moving_average_put_element(filter_moving_average_t *average, int16_t new_value); /* moving_average_t *mean, int16_t value */
extern void filter_moving_average_init(filter_moving_average_t *average, int16_t init_value);
extern void filter_moving_average_flush(filter_moving_average_t *average);

extern void filter_moving_average_float_put_element(filter_moving_average_float_t *average, float new_value);
extern void filter_moving_average_float_init(filter_moving_average_float_t *average, float init_value);
extern void filter_moving_average_flaot_flush(filter_moving_average_float_t *average);

#endif /* FILTER_H_ */
