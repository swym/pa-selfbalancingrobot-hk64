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
	uint16_t x;
	uint16_t y;
	uint16_t z;
} rotation_t;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} acceleration_t;


/* external objects            */

/* external functions          */
extern void accelerationsensor_init(uint16_t, acceleration_t *); /* uint16_t pos_multiplier, acceleration_t *accel */

extern void accelerationsensor_get_current_acceleration(acceleration_t *);
extern double accelerationsensor_get_current_position(void);

extern void accelerationsensor_calibrate_offset(void);

extern void accelerationsensor_get_offset(acceleration_t *);
extern void accelerationsensor_set_offset(acceleration_t *);

extern uint16_t accelerationsensor_get_position_multiplier(void);
extern void accelerationsensor_set_position_multiplier(uint16_t); /* uint16_t position_multiplier */


#endif /* ACCELERATIONSENSOR_H_ */
