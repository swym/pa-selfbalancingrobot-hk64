/*
 * moving_average.h
 *
 *  Created on: Nov 14, 2013
 *      Author: alexandermertens
 */


#ifndef MOVING_AVERAGE_H_
#define MOVING_AVERAGE_H_

/* *** INCLUDES ************************************************************** */

/* system headers              */ 
#include <stdint.h>

/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */

#define MOVING_AVERAGE_ELEMENT_COUNT 8

typedef struct {
	int16_t elements[MOVING_AVERAGE_ELEMENT_COUNT];
	int8_t  index;
	int32_t elements_sum;
	int16_t mean;
} moving_average_t;

/* external objects            */

/* external functions          */

#endif /* MOVING_AVERAGE_H_ */

void moving_average_simple_put_element(moving_average_t *, int16_t); /* moving_average_t *mean, int16_t value */

