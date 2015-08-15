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
	configuration.pid_robot_position.p_factor = 1;
	configuration.pid_robot_position.i_factor = 0;
	configuration.pid_robot_position.d_factor = 0;
	configuration.pid_robot_position.scalingfactor = 1;
	configuration.pid_robot_position.limit = INT16_MAX;

	configuration.pid_robot_speed.p_factor = 1;
	configuration.pid_robot_speed.i_factor = 0;
	configuration.pid_robot_speed.d_factor = 0;
	configuration.pid_robot_speed.scalingfactor = 1;
	configuration.pid_robot_speed.limit = INT16_MAX;

	configuration.pid_balance.p_factor = 1;
	configuration.pid_balance.i_factor = 0;
	configuration.pid_balance.d_factor = 0;
	configuration.pid_balance.scalingfactor = 1;
	configuration.pid_balance.limit = INT16_MAX;

	configuration.pid_motor_1_speed.p_factor = 1;
	configuration.pid_motor_1_speed.i_factor = 0;
	configuration.pid_motor_1_speed.d_factor = 0;
	configuration.pid_motor_1_speed.scalingfactor = 1;
	configuration.pid_motor_1_speed.limit = INT16_MAX;

	configuration.pid_motor_2_speed.p_factor = 1;
	configuration.pid_motor_2_speed.i_factor = 0;
	configuration.pid_motor_2_speed.d_factor = 0;
	configuration.pid_motor_2_speed.scalingfactor = 1;
	configuration.pid_motor_2_speed.limit = INT16_MAX;

	configuration.motionsensor.acceleration_offset_vector.x = 0;
	configuration.motionsensor.acceleration_offset_vector.y = 0;
	configuration.motionsensor.acceleration_offset_vector.z = 0;

	configuration.motionsensor.angularvelocity_offset_vector.x = 0;
	configuration.motionsensor.angularvelocity_offset_vector.y = 0;
	configuration.motionsensor.angularvelocity_offset_vector.z = 0;

	configuration.motionsensor.complementary_filter_ratio = 1.0;
	configuration.motionsensor.valid_acceleration_magnitude = 1.0;
	configuration.motionsensor.angle_scalingfactor = 1;

	configuration.motor_acceleration = 15;

	configuration.version = CONFIGURATION_STORAGE_VERSION;
	configuration.print_data_mode = 1;
	configuration.has_changed = true;
}

// #- GENERIC PID ----------

//should use a error type also
int16_t configuration_storage_get_pid_p_factor(pid_types_enum_t type)
{
	int16_t return_value = -1252;	//TODO: replace with real error value

	switch (type) {
		case PID_ROBOT_POSITION:
			return_value = configuration.pid_robot_position.p_factor;
			break;
		case PID_ROBOT_SPEED:
			return_value = configuration.pid_robot_speed.p_factor;
			break;
		case PID_BALANCE:
			return_value = configuration.pid_balance.p_factor;
			break;
		case PID_MOTOR_1:
			return_value = configuration.pid_motor_1_speed.p_factor;
			break;
		case PID_MOTOR_2:
			return_value = configuration.pid_motor_2_speed.p_factor;
			break;
		default:
			break;
	}

	return return_value;
}

void configuration_storage_set_pid_p_factor(pid_types_enum_t type, int16_t value)
{
	switch (type) {
		case PID_ROBOT_POSITION:
			configuration.pid_robot_position.p_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_ROBOT_SPEED:
			configuration.pid_robot_speed.p_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_BALANCE:
			configuration.pid_balance.p_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_1:
			configuration.pid_motor_1_speed.p_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_2:
			configuration.pid_motor_2_speed.p_factor =  value;
			configuration.has_changed = true;
			break;
		default:
			break;
	}
}

// -----

//should use a error type also
int16_t configuration_storage_get_pid_i_factor(pid_types_enum_t type)
{
	int16_t return_value = -1252;	//TODO: replace with real error value

	switch (type) {
		case PID_ROBOT_POSITION:
			return_value = configuration.pid_robot_position.i_factor;
			break;
		case PID_ROBOT_SPEED:
			return_value = configuration.pid_robot_speed.i_factor;
			break;
		case PID_BALANCE:
			return_value = configuration.pid_balance.i_factor;
			break;
		case PID_MOTOR_1:
			return_value = configuration.pid_motor_1_speed.i_factor;
			break;
		case PID_MOTOR_2:
			return_value = configuration.pid_motor_2_speed.i_factor;
			break;
		default:
			break;
	}

	return return_value;
}

void configuration_storage_set_pid_i_factor(pid_types_enum_t type, int16_t value)
{
	switch (type) {
		case PID_ROBOT_POSITION:
			configuration.pid_robot_position.i_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_ROBOT_SPEED:
			configuration.pid_robot_speed.i_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_BALANCE:
			configuration.pid_balance.i_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_1:
			configuration.pid_motor_1_speed.i_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_2:
			configuration.pid_motor_2_speed.i_factor =  value;
			configuration.has_changed = true;
			break;
		default:
			break;
	}
}

// -----

//should use a error type also
int16_t configuration_storage_get_pid_d_factor(pid_types_enum_t type)
{
	int16_t return_value = -1252;	//TODO: replace with real error value

	switch (type) {
		case PID_ROBOT_POSITION:
			return_value = configuration.pid_robot_position.d_factor;
			break;
		case PID_ROBOT_SPEED:
			return_value = configuration.pid_robot_speed.d_factor;
			break;
		case PID_BALANCE:
			return_value = configuration.pid_balance.d_factor;
			break;
		case PID_MOTOR_1:
			return_value = configuration.pid_motor_1_speed.d_factor;
			break;
		case PID_MOTOR_2:
			return_value = configuration.pid_motor_2_speed.d_factor;
			break;
		default:
			break;
	}

	return return_value;
}

void configuration_storage_set_pid_d_factor(pid_types_enum_t type, int16_t value)
{
	switch (type) {
		case PID_ROBOT_POSITION:
			configuration.pid_robot_position.d_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_ROBOT_SPEED:
			configuration.pid_robot_speed.d_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_BALANCE:
			configuration.pid_balance.d_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_1:
			configuration.pid_motor_1_speed.d_factor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_2:
			configuration.pid_motor_2_speed.d_factor =  value;
			configuration.has_changed = true;
			break;
		default:
			break;
	}
}

// -----

//should use a error type also
int16_t configuration_storage_get_pid_scalingfactor(pid_types_enum_t type)
{
	int16_t return_value = -1252;	//TODO: replace with real error value

	switch (type) {
		case PID_ROBOT_POSITION:
			return_value = configuration.pid_robot_position.scalingfactor;
			break;
		case PID_ROBOT_SPEED:
			return_value = configuration.pid_robot_speed.scalingfactor;
			break;
		case PID_BALANCE:
			return_value = configuration.pid_balance.scalingfactor;
			break;
		case PID_MOTOR_1:
			return_value = configuration.pid_motor_1_speed.scalingfactor;
			break;
		case PID_MOTOR_2:
			return_value = configuration.pid_motor_2_speed.scalingfactor;
			break;
		default:
			break;
	}

	return return_value;
}

void configuration_storage_set_pid_scalingfactor(pid_types_enum_t type, int16_t value)
{
	switch (type) {
		case PID_ROBOT_POSITION:
			configuration.pid_robot_position.scalingfactor =  value;
			configuration.has_changed = true;
			break;
		case PID_ROBOT_SPEED:
			configuration.pid_robot_speed.scalingfactor =  value;
			configuration.has_changed = true;
			break;
		case PID_BALANCE:
			configuration.pid_balance.scalingfactor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_1:
			configuration.pid_motor_1_speed.scalingfactor =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_2:
			configuration.pid_motor_2_speed.scalingfactor =  value;
			configuration.has_changed = true;
			break;
		default:
			break;
	}
}

// -----

//should use a error type also
int16_t configuration_storage_get_pid_limit(pid_types_enum_t type)
{
	int16_t return_value = -1252;	//TODO: replace with real error value

	switch (type) {
		case PID_ROBOT_POSITION:
			return_value = configuration.pid_robot_position.limit;
			break;
		case PID_ROBOT_SPEED:
			return_value = configuration.pid_robot_speed.limit;
			break;
		case PID_BALANCE:
			return_value = configuration.pid_balance.limit;
			break;
		case PID_MOTOR_1:
			return_value = configuration.pid_motor_1_speed.limit;
			break;
		case PID_MOTOR_2:
			return_value = configuration.pid_motor_2_speed.limit;
			break;
		default:
			break;
	}

	return return_value;
}

void configuration_storage_set_pid_limit(pid_types_enum_t type, int16_t value)
{
	switch (type) {
		case PID_ROBOT_POSITION:
			configuration.pid_robot_position.limit =  value;
			configuration.has_changed = true;
			break;
		case PID_ROBOT_SPEED:
			configuration.pid_robot_speed.limit =  value;
			configuration.has_changed = true;
			break;
		case PID_BALANCE:
			configuration.pid_balance.limit =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_1:
			configuration.pid_motor_1_speed.limit =  value;
			configuration.has_changed = true;
			break;
		case PID_MOTOR_2:
			configuration.pid_motor_2_speed.limit =  value;
			configuration.has_changed = true;
			break;
		default:
			break;
	}
}

// -----


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

float configuration_storage_get_complementary_filter_ratio(void)
{
	return configuration.motionsensor.complementary_filter_ratio;
}

void configuration_storage_set_complementary_filter_ratio(float factor)
{
	if(factor <= 1.0) {
		configuration.motionsensor.complementary_filter_ratio = factor;

		configuration.has_changed = true;
	}
}

float configuration_storage_get_valid_acceleration_magnitude(void)
{
	return configuration.motionsensor.valid_acceleration_magnitude;
}

void configuration_storage_set_valid_acceleration_magnitude(float magnitude)
{
	configuration.motionsensor.valid_acceleration_magnitude = magnitude;
	configuration.has_changed = true;
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

bool configuration_storage_config_changed(void)
{
	return configuration.has_changed;
}
