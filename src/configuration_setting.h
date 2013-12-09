/*
 * configuration_setting.h
 *
 *  Created on: Dec 5, 2013
 *      Author: alexandermertens
 */


#ifndef CONFIGURATION_SETTING_H_
#define CONFIGURATION_SETTING_H_

#include <stdint.h>
#include "accelerationsensor.h"

#define CONFIGURATION_SETTING_COUNT						6
#define CONFIGURATION_SETTING_VERSION					2
#define CONFIGURATION_SETTING_COMMENT_LENGTH			40

/*
typedef struct {
	int16_t pid_p_factor;
	int16_t pid_i_factor;
	int16_t pid_d_factor;
	uint16_t pid_scalingfactor;
	acceleration_t acceleration_offset;
	uint16_t position_multiplier;
	uint8_t setting_version;
	char comment[CONFIGURATION_SETTING_COMMENT_LENGTH];
} configuration_setting_t;
*/

typedef struct {
	int16_t pid_p_factor;
	int16_t pid_i_factor;
	int16_t pid_d_factor;
	uint16_t pid_scalingfactor;
	acceleration_t acceleration_offset;
	uint16_t position_multiplier;
	uint8_t setting_version;
	char comment[CONFIGURATION_SETTING_COMMENT_LENGTH];
} configuration_setting_t;


/* external objects            */
extern configuration_setting_t configuration_setting_data_eeprom[CONFIGURATION_SETTING_COUNT]	__attribute__ ((section (".eeprom")));
extern uint8_t configuration_setting_current_index_eeprom										__attribute__ ((section (".eeprom")));

extern configuration_setting_t configuration_setting_default;
extern configuration_setting_t configuration_setting_data[CONFIGURATION_SETTING_COUNT];
extern uint8_t configuration_setting_current_index;

#endif /* CONFIGURATION_SETTING_H_ */



