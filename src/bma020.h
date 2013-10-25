
/*
 * bma020.h
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

#ifndef BMA020_H_
#define BMA020_H_

/* *** INCLUDES ************************************************************** */
/* system headers              */ 
#include "stdbool.h"

/* local headers               */
#include "acceleration_t.h"


/* *** DECLARATIONS ********************************************************** */

/* external type and constants */

typedef struct {
	uint8_t range;
	uint8_t bandwidth;
} bma020_t;

/* external objects            */

/* external functions          */
extern void bma020_init(void);
extern void bma020_read_raw_acceleration(acceleration_t*); /* acceleration_t *raw_vector */




extern void bma020_set_new_data_int(bool);
extern bool bma020_get_new_data_int(void);

extern void bma020_set_wake_up(bool);
extern bool bma020_get_wake_up(void);

extern void bma020_set_shadow_dis(bool);
extern bool bma020_get_shadow_dis(void);

extern void bma020_set_latched_int(bool);
extern bool bma020_get_latched_int(void);

extern void bma020_set_advanced_int(bool);
extern bool bma020_get_advanced_int(void);

extern void bma020_set_spi4(bool);
extern bool bma020_get_spi4(void);

extern uint8_t  bma020_get_range(void);
extern bool     bma020_set_range(uint8_t); /* uint8_t range */

extern uint16_t bma020_get_bandwidth(void);
extern bool     bma020_set_bandwidth(uint16_t); /* uint8_t bandwidth */

/* Spaeter private */
uint8_t bma020_read_register_value(uint8_t);
void bma020_write_register_value(uint8_t, uint8_t);
int16_t bma020_read_raw_z();

#endif /* BMA020_H_ */



