/**
 * @file   bma020.h
 * @Author Alexander Mertens (alexander.mertens@stud.fh-dortmund.de)
 * @date   06.11.2013
 * @brief  This modul implements a low level driver for the bma020 acceleration
 * 	       sensor. All registers and flags are accessable through functions.
 * 	       But the readed acceleration values arn't filterd or calibrated. This
 * 	       must be implemented on a higer level of abstraction.
 *
 * Detailed description of file.
 */

#ifndef BMA020_H_
#define BMA020_H_

/* *** INCLUDES ************************************************************** */
/* system headers              */ 
#include "stdbool.h"

/* local headers               */
#include "accelerationsensor.h"


/* *** DECLARATIONS ********************************************************** */

/* external type and constants */

/* external objects            */

/* external functions          */
extern uint8_t bma020_get_chip_id();
extern uint8_t bma020_get_ml_version();
extern uint8_t bma020_get_ai_version();

extern void bma020_read_raw_acceleration(acceleration_t*); /* acceleration_t *raw_vector */

extern bool bma020_is_new_data(char); /* char axis */

extern bool bma020_get_status_hg(void);
extern bool bma020_get_status_lg(void);

extern bool bma020_get_lg_latched(void);
extern void bma020_reset_lg_latched(void);

extern bool bma020_get_hg_latched(void);
extern void bma020_reset_hg_latched(void);

extern bool bma020_get_alert_phase(void);

extern bool bma020_get_self_test_result(void);

extern void bma020_sleep(void);

extern void bma020_soft_reset(void);

extern void bma020_perform_self_test_0(void);
extern void bma020_perform_self_test_1(void);

extern void bma020_enable_spi4(bool); /* bool enable */
extern bool bma020_is_spi4(void);

extern void bma020_disable_shadow(bool); /* bool disable */
extern bool bma020_is_shadow_disabled(void);

extern void bma020_reset_interrupt(void);

extern void bma020_enable_new_data_int(bool); /* bool enable */
extern bool bma020_is_new_data_int(void);

extern void bma020_enable_lg_int(bool); /* bool enable */
extern bool bma020_is_lg_int(void);

extern void bma020_enable_hg_int(bool); /* bool enable */
extern bool bma020_is_hg_int(void);

extern void bma020_enable_advanced_int(bool); /* bool enable */
extern bool bma020_is_advanced_int(void);

extern void bma020_enable_any_motion_int(bool); /* bool enable */
extern bool bma020_is_any_motion_int(void);

extern void bma020_enable_motion_alert_int(bool); /* bool enable */
extern bool bma020_is_motion_alert_int(void);

extern void bma020_enable_latched_int(bool); /* bool enable */
extern bool bma020_is_latched_int(void);

extern void bma020_set_lg_threshold(uint8_t threshold); /* uint8_t threshold */
extern uint8_t bma020_get_lg_threshold();

extern void bma020_set_lg_duration(uint8_t); /* uint8_t duration */
extern uint8_t bma020_get_lg_duration();

extern bool bma020_set_counter_lg(uint8_t); /* uint8_t counter */
extern uint8_t bma020_get_counter_lg(void);

extern void bma020_set_lg_hysterese(uint8_t hysterese); /* uint8_t hysterese */
extern uint8_t bma020_get_lg_hysterese(void);

extern void bma020_set_hg_threshold(uint8_t); /* uint8_t threshold */
extern uint8_t bma020_get_hg_threshold();

extern void bma020_set_hg_duration(uint8_t); /* uint8_t duration */
extern uint8_t bma020_get_hg_duration();

extern bool bma020_set_counter_hg(uint8_t); /* uint8_t counter */
extern uint8_t bma020_get_counter_hg(void);

extern void bma020_set_hg_hysterese(uint8_t); /* uint8_t hysterese */
extern uint8_t bma020_get_hg_hysterese(void);

extern void bma020_set_any_motion_threshold(uint8_t); /*uint8_t threshold */
extern uint8_t bma020_get_any_motion_threshold(void);

extern bool bma020_set_any_motion_duration(uint8_t); /* uint8_t duration */
extern uint8_t bma020_get_any_motion_duration(void);

extern void bma020_enable_wake_up(bool enable);
extern bool bma020_is_wake_up(void);

extern bool bma020_set_wake_up_pause(uint16_t wake_up_time); /* uint16_t wake_up_time */
extern uint16_t bma020_get_wake_up_pause(void);

extern uint16_t bma020_get_bandwidth(void);
extern bool     bma020_set_bandwidth(uint16_t); /* uint8_t bandwidth */

extern uint8_t  bma020_get_range(void);
extern bool     bma020_set_range(uint8_t); /* uint8_t range */

extern void bma020_set_customer_reserved_1(uint8_t value);
extern uint8_t bma020_get_customer_reserved_1();

extern void bma020_set_customer_reserved_2(uint8_t value);
extern uint8_t bma020_get_customer_reserved_2();

extern void bma020_init(void);

#endif /* BMA020_H_ */



