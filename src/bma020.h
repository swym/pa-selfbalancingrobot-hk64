
/*
 * bma020.h
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

#ifndef BMA020_H_
#define BMA020_H_

/* *** INCLUDES ************************************************************** */

#define BMA020_REGISTER_CHIPID									0x00
#define BMA020_REGISTER_VERSION									0x01
#define BMA020_REGISTER_VALUE_X_LSB								0x02
#define BMA020_REGISTER_VALUE_X_MSB								0x03
#define BMA020_REGISTER_VALUE_Y_LSB								0x04
#define BMA020_REGISTER_VALUE_Y_MSB								0x05
#define BMA020_REGISTER_VALUE_Z_LSB								0x06
#define BMA020_REGISTER_VALUE_Z_MSB								0x07
#define BMA020_REGISTER_UNUSED									0x08
#define BMA020_REGISTER_STATUS									0x09
#define BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP				0x0A
#define BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE		0x0B
#define BMA020_REGISTER_SETTINGS_LG_THRESHOLD					0x0C
#define BMA020_REGISTER_SETTINGS_LG_DURATION					0x0D
#define BMA020_REGISTER_SETTINGS_HG_THRESHOLD					0x0E
#define BMA020_REGISTER_SETTINGS_HG_DURATION					0x0F
#define	BMA020_REGISTER_SETTINGS_ANY_MOTION_THRESHOLD			0x10
#define BMA020_REGISTER_SETTINGS_HYSTERESE						0x11
#define BMA020_REGISTER_CONTROL_CUSTOMER_1						0x12
#define BMA020_REGISTER_CONTROL_CUSTOMER_2						0x13
#define BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH					0x14
#define BMA020_REGISTER_CONTROL_INT								0x15


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

extern uint8_t  bma020_get_range(void);
extern bool     bma020_set_range(uint8_t); /* uint8_t range */

extern uint16_t bma020_get_bandwidth(void);
extern bool     bma020_set_bandwidth(uint16_t); /* uint8_t bandwidth */

/* Spaeter private */
uint8_t bma020_read_register_value(uint8_t);
void bma020_write_register_value(uint8_t, uint8_t);
int16_t bma020_read_raw_z();

#endif /* BMA020_H_ */



