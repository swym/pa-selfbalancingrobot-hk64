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

#include <stdio.h>



/* *** DECLARATIONS ********************************************************** */
/* import global variables */

/* local type and constants     */
static configuration_t current_configuration;
static uint8_t current_configuration_index;
static uint8_t configuration_write_index;

configuration_t configurations_eeprom[CONFIGURATION_MANAGER_CONFIG_COUNT]			__attribute__ ((section (".eeprom")));
uint8_t current_configuration_index_eeprom											__attribute__ ((section (".eeprom")));

/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */

/**
 * return true, if current configuration is valid
 * return false, if current configuration is invalid and was initialized with default values
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

	//init configuration_write_index
	configuration_write_index = (current_configuration_index + 1) %
			CONFIGURATION_MANAGER_CONFIG_COUNT;

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
		current_configuration.pid.scalingfactor = 128;

		current_configuration.accelerationsensor.acceleration_offset.x = 0;
		current_configuration.accelerationsensor.acceleration_offset.y = 0;
		current_configuration.accelerationsensor.acceleration_offset.z = 0;

		current_configuration.accelerationsensor.position_multiplier = 1;
		current_configuration.version = CONFIGURATION_MANAGER_CONFIG_VERSION;
		strncpy(current_configuration.comment, "- new -", CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH);

		current_configuration.has_changed = true;

	} else {

		current_configuration.has_changed = false;
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
	current_configuration.pid.scalingfactor = scalingfactor;
	current_configuration.has_changed = true;
}


acceleration_t * configuration_manager_current_config_get_acceleration_offset()
{
	return &current_configuration.accelerationsensor.acceleration_offset;
}


void configuration_manager_current_config_set_acceleration_offset(acceleration_t *accel)
{

	current_configuration.accelerationsensor.acceleration_offset.x = accel->x;
	current_configuration.accelerationsensor.acceleration_offset.y = accel->y;
	current_configuration.accelerationsensor.acceleration_offset.z = accel->z;

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

char * configuration_manager_current_config_get_comment(void)
{
	return current_configuration.comment;
}

void configuration_manager_current_config_set_comment(char *new_comment)
{
	strncpy(current_configuration.comment,
			new_comment,
			CONFIGURATION_MANAGER_CONFIG_COMMENT_LENGTH);

	current_configuration.has_changed = true;
}


bool configuration_manager_current_config_has_changed(void)
{
	return current_configuration.has_changed;
}

uint8_t configuration_manager_get_current_config_index(void) {
	return current_configuration_index;
}


void configuration_manager_write_config(void)
{
	if(current_configuration.has_changed) {
		current_configuration.has_changed = false;

		eeprom_write_block(&current_configuration,
				&configurations_eeprom[configuration_write_index],
				sizeof(configuration_t));

		eeprom_write_byte(&current_configuration_index_eeprom,
				configuration_write_index);
	}
}

//DEBUG Funktion
void configuration_manager_print_all_configs(void)
{
	int8_t i;
	uint8_t confs_to_print = CONFIGURATION_MANAGER_CONFIG_COUNT;
	configuration_t temp_conf;



	printf("C:%u W:%u changed:%u\n\n", current_configuration_index, configuration_write_index, current_configuration.has_changed);


	if(configuration_write_index > 0) {
		i = configuration_write_index - 1;
	} else {
		i = CONFIGURATION_MANAGER_CONFIG_COUNT - 1;
	}


    printf("           P      I      D      F      M     Ox     Oy     Oz\n\n");
	while(confs_to_print--) {

		//read current configuration from eeprom
		eeprom_read_block(&temp_conf,
				&configurations_eeprom[i],
				sizeof(configuration_t));


		if(temp_conf.version == CONFIGURATION_MANAGER_CONFIG_VERSION) {
			printf(" [%u] - \"%s\"\n", i, temp_conf.comment);


			printf("       %5i  %5i  %5i  %5u  %5u  %5i  %5i  %5i\n\n",
					temp_conf.pid.p_factor,
					temp_conf.pid.i_factor,
					temp_conf.pid.d_factor,
					temp_conf.pid.scalingfactor,
					temp_conf.accelerationsensor.position_multiplier,
					temp_conf.accelerationsensor.acceleration_offset.x,
					temp_conf.accelerationsensor.acceleration_offset.y,
					temp_conf.accelerationsensor.acceleration_offset.z);

		} else {
			printf(" [%u] - \"empty\"\n\n", i);
		}

		if( i > 0) {
			i--;
		} else {
			i = CONFIGURATION_MANAGER_CONFIG_COUNT - 1;
		}
	}
}


bool configuration_manager_select_config(uint8_t index)
{
	bool config_valid;

	configuration_t temp_conf;

	if(index <= CONFIGURATION_MANAGER_CONFIG_COUNT) {

		//read configuration from eeprom
		eeprom_read_block(&temp_conf,
				&configurations_eeprom[index],
				sizeof(configuration_t));

		//Validate before set to current config;
		if(temp_conf.version == CONFIGURATION_MANAGER_CONFIG_VERSION) {

			current_configuration_index = index;
			current_configuration = temp_conf;

			current_configuration.has_changed = true;

			config_valid = true;
		} else {
			config_valid = false;
		}
	} else {
	config_valid = false;
	}

	return config_valid;
}
