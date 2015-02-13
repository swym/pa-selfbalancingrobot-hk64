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
#define CONFIGURATION_STORAGE_VERSION			101
#define CONFIGURATION_STORAGE_COMMENT_LENGTH	60

/* * external objects            * */

typedef struct {
	int16_t p_factor;
	int16_t i_factor;
	int16_t d_factor;
	uint16_t scalingfactor;
} pid_config_t;

typedef struct {
	acceleration_t acceleration_offset;
	angularvelocity_t angularvelocity_offset;
	double complementary_filter_angularvelocity_factor;
	double complementary_filter_acceleraton_factor;
	uint16_t position_multiplier;
} motionsensor_config_t;

typedef struct {
	pid_config_t pid;
	motionsensor_config_t motionsensor;

	char comment[CONFIGURATION_STORAGE_COMMENT_LENGTH];
	uint8_t version;
	bool has_changed;
} configuration_t;

/* * external functions          * */


extern bool configuration_storage_init(void);
extern void configuration_storage_save_configuration(void);

extern int16_t configuration_storage_get_p_factor(void);
extern void configuration_storage_set_p_factor(int16_t);

extern int16_t configuration_storage_get_i_factor(void);
extern void configuration_storage_set_i_factor(int16_t);

extern int16_t configuration_storage_get_d_factor(void);
extern void configuration_storage_set_d_factor(int16_t);

extern uint16_t configuration_storage_get_scalingfactor(void);
extern void configuration_storage_set_scalingfactor(uint16_t);

extern void configuration_storage_get_acceleration_offset(acceleration_t *); /* acceleration_t *accel */
extern void configuration_storage_set_acceleration_offset(acceleration_t *); /* acceleration_t *accel */

extern void configuration_storage_get_angularvelocity_offset(angularvelocity_t *); /* angularvelocity_t *angularvelocity */
extern void configuration_storage_set_angularvelocity_offset(angularvelocity_t *); /* angularvelocity_t *angularvelocity */

extern double configuration_storage_get_complementary_filter_angularvelocity_factor(void);
extern void configuration_storage_set_complementary_filter_angularvelocity_factor(double);

extern double configuration_storage_get_complementary_filter_acceleraton_factor(void);
extern void configuration_storage_set_complementary_filter_acceleraton_factor(double);

extern uint16_t configuration_storage_get_position_multiplier(void);
extern void configuration_storage_set_position_multiplier(uint16_t);

extern char * configuration_storage_get_comment(void);
extern void configuration_storage_set_comment(char *);

extern bool configuration_storage_config_changed(void);


#endif /* CONFIGURATION_STORAGE_H_ */
