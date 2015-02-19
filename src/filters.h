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

#define MOVING_AVERAGE_ELEMENT_COUNT 8
#define WEIGHTED_AVERAGE_ELEMENT_COUNT 5

typedef struct {
	int16_t elements[MOVING_AVERAGE_ELEMENT_COUNT];
	int8_t  index;
	int32_t elements_sum;
	int16_t mean;
} moving_average_t;

typedef struct {
	int16_t elements[WEIGHTED_AVERAGE_ELEMENT_COUNT];
	int16_t mean;								//sum of all elements and their weights
} weighted_average_t;

typedef struct {
	int16_t	smoothed;
	uint8_t factor;
} smooth_t;

/* * external objects            * */

/* * external functions          * */

void filters_moving_average_put_element(moving_average_t *average, int16_t value); /* moving_average_t *mean, int16_t value */
void filters_weighted_average_put_element(weighted_average_t *average, int16_t value); /* moving_average_t *mean, int16_t value */
void filters_smooth_put_element(smooth_t * average, int16_t value);

#endif /* FILTERS_H_ */
