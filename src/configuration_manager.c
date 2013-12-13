/*
 * configuration_manager.c
 *
 *  Created on: Dec 12, 2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "configuration_manager.h"

#include <avr/eeprom.h>
#include <string.h>



/* *** DECLARATIONS ********************************************************** */
/* import global variables */

/* local type and constants     */
static configuration_t current_configuration;
static uint8_t current_configuration_index;

configuration_t configurations_eeprom[CONFIGURATION_MANAGER_CONFIG_COUNT]			__attribute__ ((section (".eeprom")));
uint8_t current_configuration_index_eeprom											__attribute__ ((section (".eeprom")));

/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */

/**
 * return true, if current configuration is valid
 * return false, if current configuration is invalid and was initialised with default values
 * @return
 */
bool configuration_manager_init(void)
{
	bool current_config_valid = true;

	//read index of current setting
	current_configuration_index = eeprom_read_byte(&current_configuration_index_eeprom);

	//validate index; if invalid, set zero, return false
	if(current_configuration_index >= CONFIGURATION_MANAGER_CONFIG_COUNT) {
		current_configuration_index = 0;

		current_config_valid = false;
	}

	//read current configuration from eeprom
	eeprom_read_block(&current_configuration,
			&configurations_eeprom[current_configuration_index],
			sizeof(configuration_t));


	//validate configuration against the current version number. if invalid set defaults
	if(current_config_valid == false &&
			current_configuration.version != CONFIGURATION_MANAGER_CONFIG_VERSION) {

		current_configuration.pid.p_factor = 0;
		current_configuration.pid.i_factor = 0;
		current_configuration.pid.d_factor = 0;
		current_configuration.pid.scalingfactor = 1;

		current_configuration.accelerationsensor.acceleration_offset.x = 0;
		current_configuration.accelerationsensor.acceleration_offset.y = 0;
		current_configuration.accelerationsensor.acceleration_offset.z = 0;

		current_configuration.accelerationsensor.position_multiplier = 1;
		current_configuration.version = CONFIGURATION_MANAGER_CONFIG_VERSION;
		strcpy(current_configuration.comment, "- new -");
	}


	return current_config_valid;
}

int16_t configuration_manager_current_config_get_p_factor(void)
{
	return current_configuration.pid.p_factor;
}

void configuration_manager_current_config_set_p_factor(int16_t p_factor)
{
	current_configuration.pid.p_factor =  p_factor;
	current_configuration.has_changed = true;
}



int16_t configuration_manager_current_config_get_i_factor(void)
{
	return current_configuration.pid.i_factor;
}

void configuration_manager_current_config_set_i_factor(int16_t i_factor)
{
	current_configuration.pid.i_factor =  i_factor;
	current_configuration.has_changed = true;
}



int16_t configuration_manager_current_config_get_d_factor(void)
{
	return current_configuration.pid.d_factor;
}

void configuration_manager_current_config_set_d_factor(int16_t d_factor)
{
	current_configuration.pid.d_factor =  d_factor;
	current_configuration.has_changed = true;
}


uint16_t configuration_manager_current_config_get_scalingfactor(void)
{
	return current_configuration.pid.scalingfactor;
}

void configuration_manager_current_config_set_scalingfactor(uint16_t scalingfactor)
{
	current_configuration.pid.scalingfactor =  scalingfactor;
	current_configuration.has_changed = true;
}


void configuration_manager_current_config_get_acceleration_offset(acceleration_t *accel)
{
	memcpy(accel,
			&current_configuration.accelerationsensor.acceleration_offset,
			sizeof(acceleration_t));
}


void configuration_manager_current_config_set_acceleration_offset(acceleration_t *accel)
{

	memcpy(&current_configuration.accelerationsensor.acceleration_offset,
			accel,
			sizeof(acceleration_t));

	current_configuration.has_changed = true;
}


uint16_t configuration_manager_current_config_get_position_multiplier(void)
{
	return current_configuration.accelerationsensor.position_multiplier;
}

void configuration_manager_current_config_set_position_multiplier(uint16_t multiplier)
{
	current_configuration.accelerationsensor.position_multiplier = multiplier;
	current_configuration.has_changed = true;
}


void configuration_manager_write_config(void)
{
	if(current_configuration.has_changed) {
		eeprom_write_block(&current_configuration,
				&configurations_eeprom[current_configuration_index],
				sizeof(configuration_t));

		eeprom_write_byte(&current_configuration_index_eeprom,
				current_configuration_index);

		current_configuration.has_changed = false;
	}
}
