/*
 * configuration_storage.h
 *
 *  Created on: Jun 29, 2014
 *      Author: alexandermertens
 */


#ifndef CONFIGURATION_STORAGE_H_
#define CONFIGURATION_STORAGE_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdbool.h>

/* * local headers               * */
#include "motionsensor.h"

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define CONFIGURATION_STORAGE_VERSION			113


/* * external objects            * */

typedef enum  {
	PRINT_NONE,
	PRINT_TICKER,
	PRINT_DATA_ANGLEPID,
	PRINT_DATA_ALL_RAW,
	PRINT_DATA_ALL_FILTERED,
	PRINT_DATA_REALLY_ALL_FILTERED,
	FINAL_print_data_enum_t_ENTRY

} print_data_enum_t;

typedef struct {
	int16_t p_factor;
	int16_t i_factor;
	int16_t d_factor;
	uint16_t pid_scalingfactor;
	int16_t pid_limit;
} pid_config_t;

typedef struct {
	acceleration_vector_t acceleration_offset_vector;
	angularvelocity_vector_t angularvelocity_offset_vector;
	float complementary_filter_ratio;
	float valid_acceleration_magnitude;
	uint16_t angle_scalingfactor;
} motionsensor_config_t;

typedef struct {
	pid_config_t pid_robot_position;
	pid_config_t pid_robot_speed;
	pid_config_t pid_balance;
	pid_config_t pid_motor_1_speed;
	pid_config_t pid_motor_2_speed;

	motionsensor_config_t motionsensor;
	uint8_t		 motor_acceleration;
	uint8_t version;
	print_data_enum_t print_data_mode;
	bool has_changed;
} configuration_t;

/* * external functions          * */


extern bool configuration_storage_init(void);
extern void configuration_storage_save_configuration(void);
extern void configuration_storage_reset_configuration(void);

// getter/setter

// #- PID ROBOT POSITION ----------

extern int16_t configuration_storage_get_pid_robot_position_p_factor(void);
extern void configuration_storage_set_pid_robot_position_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_robot_position_i_factor(void);
extern void configuration_storage_set_pid_robot_position_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_robot_position_d_factor(void);
extern void configuration_storage_set_pid_robot_position_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_robot_position_scalingfactor(void);
extern void configuration_storage_set_pid_robot_position_scalingfactor(uint16_t);

extern uint16_t configuration_storage_get_pid_robot_position_limit(void);
extern void configuration_storage_set_pid_robot_position_limit(uint16_t);


// #- PID ROBOT SPEED ----------

extern int16_t configuration_storage_get_pid_robot_speed_p_factor(void);
extern void configuration_storage_set_pid_robot_speed_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_robot_speed_i_factor(void);
extern void configuration_storage_set_pid_robot_speed_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_robot_speed_d_factor(void);
extern void configuration_storage_set_pid_robot_speed_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_robot_speed_scalingfactor(void);
extern void configuration_storage_set_pid_robot_speed_scalingfactor(uint16_t);

extern uint16_t configuration_storage_get_pid_robot_speed_limit(void);
extern void configuration_storage_set_pid_robot_speed_limit(uint16_t);


// #- PID BALANCE ----------

extern int16_t configuration_storage_get_pid_balance_p_factor(void);
extern void configuration_storage_set_pid_balance_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_balance_i_factor(void);
extern void configuration_storage_set_pid_balance_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_balance_d_factor(void);
extern void configuration_storage_set_pid_balance_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_balance_scalingfactor(void);
extern void configuration_storage_set_pid_balance_scalingfactor(uint16_t);

extern uint16_t configuration_storage_get_pid_balance_limit(void);
extern void configuration_storage_set_pid_balance_limit(uint16_t);


// #- PID MOTOR SPEED 1 ----------

extern int16_t configuration_storage_get_pid_motor_one_speed_p_factor(void);
extern void configuration_storage_set_pid_motor_one_speed_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_motor_one_speed_i_factor(void);
extern void configuration_storage_set_pid_motor_one_speed_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_motor_one_speed_d_factor(void);
extern void configuration_storage_set_pid_motor_one_speed_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_motor_one_speed_scalingfactor(void);
extern void configuration_storage_set_pid_motor_one_speed_scalingfactor(uint16_t);

extern uint16_t configuration_storage_get_pid_motor_one_speed_limit(void);
extern void configuration_storage_set_pid_motor_one_speed_limit(uint16_t);


// #- PID MOTOR SPEED 2 ----------

extern int16_t configuration_storage_get_pid_motor_two_speed_p_factor(void);
extern void configuration_storage_set_pid_motor_two_speed_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_motor_two_speed_i_factor(void);
extern void configuration_storage_set_pid_motor_two_speed_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_motor_two_speed_d_factor(void);
extern void configuration_storage_set_pid_motor_two_speed_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_motor_two_speed_scalingfactor(void);
extern void configuration_storage_set_pid_motor_two_speed_scalingfactor(uint16_t);

extern uint16_t configuration_storage_get_pid_motor_two_speed_limit(void);
extern void configuration_storage_set_pid_motor_two_speed_limit(uint16_t);

// #- MOTION SENSOR ----------

extern void configuration_storage_get_acceleration_offset_vector(acceleration_vector_t * accel_v);
extern void configuration_storage_set_acceleration_offset_vector(acceleration_vector_t * angular_v);

extern void configuration_storage_get_angularvelocity_offset_vector(angularvelocity_vector_t * accel_v);
extern void configuration_storage_set_angularvelocity_offset_vector(angularvelocity_vector_t * angular_v);

extern float configuration_storage_get_complementary_filter_ratio(void);
extern void configuration_storage_set_complementary_filter_ratio(float ratio);

extern float configuration_storage_get_valid_acceleration_magnitude(void);
extern void configuration_storage_set_valid_acceleration_magnitude(float magnitude);

extern uint16_t configuration_storage_get_angle_scalingfactor(void);
extern void configuration_storage_set_angle_scalingfactor(uint16_t);

// #- MISC ----------

extern uint8_t configuration_storage_get_motor_acceleration(void);
extern void configuration_storage_set_motor_acceleration(uint8_t);

extern void configuration_storage_set_print_data_mode(print_data_enum_t mode);
extern print_data_enum_t configuration_storage_get_print_data_mode(void);

extern bool configuration_storage_config_has_changed(void);


#endif /* CONFIGURATION_STORAGE_H_ */
