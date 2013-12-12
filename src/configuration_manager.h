/*
 * configuration_manager.h
 *
 *  Created on: Dec 12, 2013
 *      Author: alexandermertens
 */

#ifndef CONFIGURATION_MANAGER_H_
#define CONFIGURATION_MANAGER_H_


#include <stdint.h>
#include <stdbool.h>

#define CONFIGURATION_MANAGER_CONFIG_COUNT					6
#define CONFIGURATION_MANAGER_CONFIG_VERSION				3
#define CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH			60

typedef struct {
	pid_configuration_t pid;
	accelerationsensor_config_t accelerationsensor;

	uint8_t config_version;
	char comment[CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH];
	bool has_changed;

} configuration_t;

typedef struct {
	int16_t p_factor;
	int16_t i_factor;
	int16_t d_factor;
	uint16_t scalingfactor;
} pid_configuration_t;

typedef struct {
	acceleration_t acceleration_offset;
	uint16_t position_multiplier;
} accelerationsensor_config_t;


#endif /* CONFIGURATION_MANAGER_H_ */
