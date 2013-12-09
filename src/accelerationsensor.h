/*
 * acceleration_t.h
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

#ifndef ACCELERATIONSENSOR_H_
#define ACCELERATIONSENSOR_H_

/* *** INCLUDES ************************************************************** */

/* system headers              */
#include <stdint.h>

/* local headers               */

/* *** DECLARATIONS ********************************************************** */

/* external type and constants */

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} acceleration_t;

/* external objects            */

/* external functions          */
extern void accelerationsensor_get_current_acceleration(acceleration_t *);
extern void accelerationsensor_init(void);

extern void accelerationsensor_calibrate_offset(void);
extern void accelerationsensor_get_offset(acceleration_t *);
extern void accelerationsensor_set_offset(acceleration_t *);


#endif /* ACCELERATIONSENSOR_H_ */
