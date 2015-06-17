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
#define CONFIGURATION_STORAGE_VERSION			112
#define CONFIGURATION_STORAGE_COMMENT_LENGTH	20

#define CONFIGURATION_STORAGE_RUN_MODE_NORMAL	0
#define CONFIGURATION_STORAGE_RUN_MODE_DEBUG	1

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
} pid_config_t;

typedef struct {
	acceleration_vector_t acceleration_offset_vector;
	angularvelocity_vector_t angularvelocity_offset_vector;
	float complementary_filter_ratio;
	float valid_acceleration_magnitude;
	uint16_t angle_scalingfactor;
} motionsensor_config_t;

typedef struct {
	pid_config_t pid_robot_pos;
	pid_config_t pid_balance; //TODO: Rename to balance...
	//pid_config_t pid_edge;   //TODO: rename to balance...
	pid_config_t pid_speed_motor;

	//uint16_t  pid_edge_angle; //TODO: use	motionsensor_angle_t
	motionsensor_angle_t  angle_stable;

	motionsensor_config_t motionsensor;
	uint8_t		 motor_acceleration;
	char comment[CONFIGURATION_STORAGE_COMMENT_LENGTH];
	uint8_t version;
	print_data_enum_t print_data_mode;
	bool has_changed;
} configuration_t;

/* * external functions          * */


extern bool configuration_storage_init(void);
extern void configuration_storage_save_configuration(void);
extern void configuration_storage_reset_configuration(void);

// getter/setter
// #-----------

extern int16_t configuration_storage_get_pid_robot_pos_p_factor(void);
extern void configuration_storage_set_pid_robot_pos_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_robot_pos_i_factor(void);
extern void configuration_storage_set_pid_robot_pos_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_robot_pos_d_factor(void);
extern void configuration_storage_set_pid_robot_pos_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_robot_pos_scalingfactor(void);
extern void configuration_storage_set_pid_robot_pos_scalingfactor(uint16_t);

// #-----------

extern int16_t configuration_storage_get_pid_balance_p_factor(void);
extern void configuration_storage_set_pid_balance_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_balance_i_factor(void);
extern void configuration_storage_set_pid_balance_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_balance_d_factor(void);
extern void configuration_storage_set_pid_balance_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_balance_scalingfactor(void);
extern void configuration_storage_set_pid_balance_scalingfactor(uint16_t);

// #-----------

extern int16_t configuration_storage_get_pid_speed_motor_p_factor(void);
extern void configuration_storage_set_pid_speed_motor_p_factor(int16_t);

extern int16_t configuration_storage_get_pid_speed_motor_i_factor(void);
extern void configuration_storage_set_pid_speed_motor_i_factor(int16_t);

extern int16_t configuration_storage_get_pid_speed_motor_d_factor(void);
extern void configuration_storage_set_pid_speed_motor_d_factor(int16_t);

extern uint16_t configuration_storage_get_pid_speed_motor_scalingfactor(void);
extern void configuration_storage_set_pid_speed_motor_scalingfactor(uint16_t);

// #-----------


//extern uint16_t configuration_storage_get_pid_edge_angle(void);
//extern void configuration_storage_set_pid_edge_angle(uint16_t);

extern motionsensor_angle_t configuration_storage_get_angle_stable(void);
extern void configuration_storage_set_angle_stable(motionsensor_angle_t);

// #-----------

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

// #-----------

extern uint8_t configuration_storage_get_motor_acceleration(void);
extern void configuration_storage_set_motor_acceleration(uint8_t);

extern char * configuration_storage_get_comment(void);
extern void configuration_storage_set_comment(char * comment);

extern void configuration_storage_set_print_data_mode(print_data_enum_t mode);
extern print_data_enum_t configuration_storage_get_print_data_mode(void);

extern bool configuration_storage_config_has_changed(void);


#endif /* CONFIGURATION_STORAGE_H_ */
