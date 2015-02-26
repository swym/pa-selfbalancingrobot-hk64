/*
 * filters.h
 *
 *  Created on: Jul 16, 2014
 *      Author: alexandermertens
 */


#ifndef FILTERS_H_
#define FILTERS_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdint.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define FILTER_MOVING_GENERIC_WEIGHTS_COUNT		8
#define FILTER_MOVING_AVERAGE_ELEMENT_COUNT		8

typedef struct {
	int16_t avg;
	int16_t elements[FILTER_MOVING_AVERAGE_ELEMENT_COUNT];
	int8_t  index;
	int32_t elements_sum;
} filters_moving_average_t;

typedef struct {
	int16_t avg;
	int16_t elements[FILTER_MOVING_GENERIC_WEIGHTS_COUNT];
	uint8_t weights[FILTER_MOVING_GENERIC_WEIGHTS_COUNT];
	uint8_t weights_count;
	uint8_t weights_sum;
} filters_moving_generic_average_t;

/* * external objects            * */

/* * external functions          * */
extern void filters_moving_generic_average_put_element(filters_moving_generic_average_t * average, int16_t new_value);
extern void filters_moving_generic_average_init(filters_moving_generic_average_t * average, uint8_t * weights, int16_t init_value);
extern void filters_moving_generic_average_flush(filters_moving_generic_average_t * average);

extern void filters_moving_average_put_element(filters_moving_average_t *average, int16_t new_value); /* moving_average_t *mean, int16_t value */
extern void filters_moving_average_init(filters_moving_average_t *average, int16_t init_value);
extern void filters_moving_average_flush(filters_moving_average_t *average);


#endif /* FILTERS_H_ */
