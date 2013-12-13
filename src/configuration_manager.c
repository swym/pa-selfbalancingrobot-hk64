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

		current_configuration.pid.scalingfactor = 1;
		current_configuration.accelerationsensor.position_multiplier = 1;
		current_configuration.version = CONFIGURATION_MANAGER_CONFIG_VERSION;
		strcpy(current_configuration.comment, "- new -");
	}


	return current_config_valid;
}
