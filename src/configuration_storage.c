/*
 * configuration_storage.c
 *
 *  Created on: Jun 29, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "configuration_storage.h"

/* * system headers              * */
#include <avr/eeprom.h>
#include <string.h>

#include <stdio.h>

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
configuration_t configuration_eeprom	__attribute__ ((section (".eeprom")));

/* * local objects               * */
static configuration_t configuration;

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */
bool configuration_storage_get_init()
{
	bool returnvalue = true;

	//read current configuration from eeprom
	eeprom_read_block(&configuration,
			&configuration_eeprom,
			sizeof(configuration_t));

	// check configuration against the current version number.
	// if invalid set defaults
	if(configuration.version != CONFIGURATION_STORAGE_VERSION) {

		configuration.pid.p_factor = 1;
		configuration.pid.i_factor = 1;
		configuration.pid.d_factor = 1;
		configuration.pid.scalingfactor = 128;

		configuration.motionsensor.acceleration_offset.x = 0;
		configuration.motionsensor.acceleration_offset.y = 0;
		configuration.motionsensor.acceleration_offset.z = 0;
		configuration.motionsensor.position_multiplier = 1.0;

		strncpy(configuration.comment, "- new -", CONFIGURATION_STORAGE_COMMENT_LENGTH);
		configuration.version = CONFIGURATION_STORAGE_VERSION;
		configuration.has_changed = true;

		returnvalue = false;
	}

	return returnvalue;
}

void configuration_storage_save_configuration()
{
	printf("try to save data\n");
	printf("accel.x: %d\n", configuration.motionsensor.acceleration_offset.x);

	if(configuration.has_changed) {
		eeprom_write_block(&configuration,
				&configuration_eeprom,
				sizeof(configuration_t));
	}
}

int16_t configuration_storage_get_p_factor(void)
{
	return configuration.pid.p_factor;
}

void configuration_storage_set_p_factor(int16_t p_factor)
{
	configuration.pid.p_factor =  p_factor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_i_factor(void)
{
	return configuration.pid.i_factor;
}

void configuration_storage_set_i_factor(int16_t i_factor)
{
	configuration.pid.i_factor =  i_factor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_d_factor(void)
{
	return configuration.pid.d_factor;
}

void configuration_storage_set_d_factor(int16_t d_factor)
{
	configuration.pid.d_factor =  d_factor;
	configuration.has_changed = true;
}


void configuration_storage_set_scalingfactor(uint16_t scalingfactor)
{
	configuration.pid.scalingfactor = scalingfactor;
	configuration.has_changed = true;
}


acceleration_t * configuration_storage_get_acceleration_offset()
{
	return &configuration.motionsensor.acceleration_offset;
}


void configuration_storage_set_acceleration_offset(acceleration_t *accel)
{

	configuration.motionsensor.acceleration_offset.x = accel->x;
	configuration.motionsensor.acceleration_offset.y = accel->y;
	configuration.motionsensor.acceleration_offset.z = accel->z;

	configuration.has_changed = true;
}


uint16_t configuration_storage_get_position_multiplier(void)
{
	return configuration.motionsensor.position_multiplier;
}

void configuration_storage_set_position_multiplier(uint16_t multiplier)
{
	configuration.motionsensor.position_multiplier = multiplier;
	configuration.has_changed = true;
}


char * configuration_storage_get_comment(void)
{
	return configuration.comment;
}

void configuration_storage_set_comment(char *new_comment)
{
	strncpy(configuration.comment,
			new_comment,
			CONFIGURATION_STORAGE_COMMENT_LENGTH);

	configuration.has_changed = true;
}


bool configuration_storage_config_changed(void)
{
	return configuration.has_changed;
}
