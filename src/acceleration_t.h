/*
 * acceleration_t.h
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

#ifndef ACCELERATION_T_H_
#define ACCELERATION_T_H_

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
extern void acceleration_get_current_acceleration(acceleration_t *);
extern void acceleration_init(void);

extern void acceleration_calibrate_offset(void);
extern void acceleration_get_offset(acceleration_t *);


#endif /* ACCELERATION_T_H_ */
