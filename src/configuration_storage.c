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
bool configuration_storage_init()
{
	bool returnvalue = true;

	//read current configuration from eeprom
	eeprom_read_block(&configuration,
			&configuration_eeprom,
			sizeof(configuration_t));

	// check configuration against the current version number.
	// if invalid set defaults
	if(configuration.version != CONFIGURATION_STORAGE_VERSION) {

		configuration_storage_reset_configuration();
		returnvalue = false;
	}

	return returnvalue;
}

void configuration_storage_save_configuration(void)
{
	if(configuration.has_changed) {
		eeprom_write_block(&configuration,
				&configuration_eeprom,
				sizeof(configuration_t));
	}
}

void configuration_storage_reset_configuration(void)
{
	configuration.pid_center.p_factor = 0;
	configuration.pid_center.i_factor = 0;
	configuration.pid_center.d_factor = 0;
	configuration.pid_center.pid_scalingfactor = 1;

	configuration.pid_edge.p_factor = 0;
	configuration.pid_edge.i_factor = 0;
	configuration.pid_edge.d_factor = 0;
	configuration.pid_edge.pid_scalingfactor = 1;

	configuration.pid_edge_angle = INT16_MAX;

	configuration.motionsensor.acceleration_offset_vector.x = 0;
	configuration.motionsensor.acceleration_offset_vector.y = 0;
	configuration.motionsensor.acceleration_offset_vector.z = 0;

	configuration.motionsensor.angularvelocity_offset_vector.x = 0;
	configuration.motionsensor.angularvelocity_offset_vector.y = 0;
	configuration.motionsensor.angularvelocity_offset_vector.z = 0;

	configuration.motionsensor.complementary_filter_ratio = 1000;

	configuration.motionsensor.angle_scalingfactor = 1;

	configuration.motor_acceleration = 15;

	strncpy(configuration.comment, "- new -", CONFIGURATION_STORAGE_COMMENT_LENGTH);
	configuration.version = CONFIGURATION_STORAGE_VERSION;
	configuration.print_data_mode = 1;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_pid_center_p_factor(void)
{
	return configuration.pid_center.p_factor;
}

void configuration_storage_set_pid_center_p_factor(int16_t p_factor)
{
	configuration.pid_center.p_factor =  p_factor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_pid_center_i_factor(void)
{
	return configuration.pid_center.i_factor;
}

void configuration_storage_set_pid_center_i_factor(int16_t i_factor)
{
	configuration.pid_center.i_factor =  i_factor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_pid_center_d_factor(void)
{
	return configuration.pid_center.d_factor;
}

void configuration_storage_set_pid_center_d_factor(int16_t d_factor)
{
	configuration.pid_center.d_factor =  d_factor;
	configuration.has_changed = true;
}

uint16_t configuration_storage_get_pid_center_scalingfactor(void)
{
	return configuration.pid_center.pid_scalingfactor;
}

void configuration_storage_set_pid_center_scalingfactor(uint16_t pid_scalingfactor)
{
	configuration.pid_center.pid_scalingfactor = pid_scalingfactor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_pid_edge_p_factor(void)
{
	return configuration.pid_edge.p_factor;
}

void configuration_storage_set_pid_edge_p_factor(int16_t p_factor)
{
	configuration.pid_edge.p_factor =  p_factor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_pid_edge_i_factor(void)
{
	return configuration.pid_edge.i_factor;
}

void configuration_storage_set_pid_edge_i_factor(int16_t i_factor)
{
	configuration.pid_edge.i_factor =  i_factor;
	configuration.has_changed = true;
}

int16_t configuration_storage_get_pid_edge_d_factor(void)
{
	return configuration.pid_edge.d_factor;
}

void configuration_storage_set_pid_edge_d_factor(int16_t d_factor)
{
	configuration.pid_edge.d_factor =  d_factor;
	configuration.has_changed = true;
}

uint16_t configuration_storage_get_pid_edge_scalingfactor(void)
{
	return configuration.pid_edge.pid_scalingfactor;
}

void configuration_storage_set_pid_edge_scalingfactor(uint16_t pid_scalingfactor)
{
	configuration.pid_edge.pid_scalingfactor = pid_scalingfactor;
	configuration.has_changed = true;
}

uint16_t configuration_storage_get_pid_edge_angle(void)
{
	return configuration.pid_edge_angle;
}

void configuration_storage_set_pid_edge_angle(uint16_t angle)
{
	configuration.pid_edge_angle = angle;
	configuration.has_changed = true;
}

void configuration_storage_get_acceleration_offset_vector(acceleration_vector_t * accel_v)
{
	accel_v->x = configuration.motionsensor.acceleration_offset_vector.x;
	accel_v->y = configuration.motionsensor.acceleration_offset_vector.y;
	accel_v->z = configuration.motionsensor.acceleration_offset_vector.z;
}


void configuration_storage_set_acceleration_offset_vector(acceleration_vector_t * accel_v)
{

	configuration.motionsensor.acceleration_offset_vector.x = accel_v->x;
	configuration.motionsensor.acceleration_offset_vector.y = accel_v->y;
	configuration.motionsensor.acceleration_offset_vector.z = accel_v->z;

	configuration.has_changed = true;
}

void configuration_storage_get_angularvelocity_offset_vector(angularvelocity_vector_t * angular_v)
{
	angular_v->x = configuration.motionsensor.angularvelocity_offset_vector.x;
	angular_v->y = configuration.motionsensor.angularvelocity_offset_vector.y;
	angular_v->z = configuration.motionsensor.angularvelocity_offset_vector.z;
}


void configuration_storage_set_angularvelocity_offset_vector(angularvelocity_vector_t * angular_v)
{

	configuration.motionsensor.angularvelocity_offset_vector.x = angular_v->x;
	configuration.motionsensor.angularvelocity_offset_vector.y = angular_v->y;
	configuration.motionsensor.angularvelocity_offset_vector.z = angular_v->z;

	configuration.has_changed = true;
}

uint16_t configuration_storage_get_complementary_filter_ratio(void)
{
	return configuration.motionsensor.complementary_filter_ratio;
}

void configuration_storage_set_complementary_filter_ratio(uint16_t factor)
{
	if(factor <= 1000) {
		configuration.motionsensor.complementary_filter_ratio = factor;

		configuration.has_changed = true;
	}
}

uint16_t configuration_storage_get_angle_scalingfactor(void)
{
	return configuration.motionsensor.angle_scalingfactor;
}

void configuration_storage_set_angle_scalingfactor(uint16_t as)
{
	configuration.motionsensor.angle_scalingfactor = as;
	configuration.has_changed = true;
}


char * configuration_storage_get_comment(void)
{
	return configuration.comment;
}

uint8_t configuration_storage_get_motor_acceleration(void)
{
	return configuration.motor_acceleration;
}

void configuration_storage_set_print_data_mode(print_data_enum_t mode)
{
	if(mode < FINAL_print_data_enum_t_ENTRY) {
		configuration.print_data_mode = mode;
	}
}
print_data_enum_t configuration_storage_get_print_data_mode(void)
{
	return configuration.print_data_mode;
}

extern void configuration_storage_set_motor_acceleration(uint8_t accel)
{
	configuration.motor_acceleration = accel;
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
