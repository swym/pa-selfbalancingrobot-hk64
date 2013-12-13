/**
 * @file   test_bma020.h
 * @Author Alexander Mertens (alexander.mertens@stud.fh-dortmund.de)
 * @date   11.11.2013
 * @brief  This modul implements a set of tests, which checks the functionality
 *         of the low level driver modul "bma020.c".
 *
 * Detailed description of file.
 */

/* *** INCLUDES ************************************************************** */
/* MODUL HEADER */
#include "test_bma020.h"

/* SYSTEM INCLUDES */
#include <util/delay.h>

/* LOCAL INCLUDES */
#include "../accelerationsensor.h"
/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void test_bma020_register0x15(bool, bool); /* bool reset_bma020, bool print_logical */
static void test_bma020_register0x14(bool, bool); /* bool reset_bma020, bool print_logical */
static void test_bma020_register0x11(bool, bool); /* bool reset_bma020, bool print_logical */
static void test_bma020_register0x0B(bool, bool); /* bool reset_bma020, bool print_logical */
static void test_bma020_read_and_sleep(void);

/* *** FUNCTION DEFINITIONS ************************************************** */


void test_bma020_settings()
{
//	test_bma020_register0x15(true, true);
//	test_bma020_register0x14(true, true);
//	test_bma020_register0x11(false, true);
//	test_bma020_register0x0B(true, true);

	test_bma020_read_and_sleep();
}

/**
 * Test flags and values in register 0x15
 * @param reset_bma020 - if true, resets the bma020, so that the defaults will be load
 * @param print_logical - if true, getters will exectutes and the results will be printed
 */
void test_bma020_register0x15(bool reset_bma020, bool print_logical)
{

	uint8_t slave_address = (0x70 >> 1);
	uint8_t register_address = 0x15;
	uint8_t val;

	if(reset_bma020) {
		bma020_soft_reset();
		_delay_ms(10);
	}

	printf("read old values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	printf("set new values through setters\n");
	bma020_enable_spi4(true);
	bma020_enable_advanced_int(true);
	bma020_enable_new_data_int(false);
	bma020_enable_latched_int(true);
	bma020_disable_shadow(true);
	bma020_set_wake_up_pause(2561);
	bma020_enable_wake_up(false);

	printf("read new values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	if(print_logical) {
		printf("SPI4:              %d\n", bma020_is_spi4());
		printf("ADV_INT:           %d\n", bma020_is_advanced_int());
		printf("NEW_DATA_INT:      %d\n", bma020_is_new_data_int());
		printf("latch_INT:         %d\n", bma020_is_latched_int());
		printf("shadow disabled:   %d\n", bma020_is_shadow_disabled());
		printf("wake_up_pause:     %d\n", bma020_get_wake_up_pause());
		printf("wake_up_enabled:   %d\n", bma020_is_wake_up());
	}
}

/**
 * Test flags and values in register 0x14
 * @param reset_bma020 - if true, resets the bma020, so that the defaults will be load
 * @param print_logical - if true, getters will exectutes and the results will be printed
 */
void test_bma020_register0x14(bool reset_bma020, bool print_logical)
{
	uint8_t slave_address = (0x70 >> 1);
	uint8_t register_address = 0x14;
	uint8_t val;

	if(reset_bma020) {
		bma020_soft_reset();
		_delay_ms(10);
	}

	printf("read old values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	printf("set new values through setters\n");

	bma020_set_range(10);
	bma020_set_bandwidth(1501);

	printf("read new values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	if(print_logical) {
		printf("Range:             %d\n", bma020_get_range());
		printf("Bandwidth:         %d\n", bma020_get_bandwidth());
	}
}

/**
 * Test flags and values in register 0x11
 * @param reset_bma020 - if true, resets the bma020, so that the defaults will be load
 * @param print_logical - if true, getters will exectutes and the results will be printed
 */
void test_bma020_register0x11(bool reset_bma020, bool print_logical) {

	uint8_t slave_address = (0x70 >> 1);
	uint8_t register_address = 0x11;
	uint8_t val;

	if(reset_bma020) {
		bma020_soft_reset();
		_delay_ms(10);
	}

	printf("read old values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	printf("set new values through setters\n");

	bma020_set_any_motion_duration(3);
	bma020_set_hg_hysterese(4);
	bma020_set_lg_hysterese(-12);

	printf("read new values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	if(print_logical) {
		printf("Any_Motion_dur   %d\n", bma020_get_any_motion_duration());
		printf("HG_hysterese     %d\n", bma020_get_hg_hysterese());
		printf("LG_hysterese     %d\n", bma020_get_lg_hysterese());
	}
}

/**
 * Test flags and values in register 0x0B
 * @param reset_bma020 - if true, resets the bma020, so that the defaults will be load
 * @param print_logical - if true, getters will exectutes and the results will be printed
 */
void test_bma020_register0x0B(bool reset_bma020, bool print_logical) {

	uint8_t slave_address = (0x70 >> 1);
	uint8_t register_address = 0x0B;
	uint8_t val;

	if(reset_bma020) {
		bma020_soft_reset();
		_delay_ms(10);
	}

	printf("read old values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	printf("set new values through setters\n");

	bma020_enable_motion_alert_int(true);
	bma020_enable_any_motion_int(false);
	bma020_set_counter_hg(1);
	bma020_set_counter_lg(1);
	bma020_enable_hg_int(true);
	bma020_enable_lg_int(false);


	printf("read new values\n");
	val = twi_master_read_register(slave_address, register_address);
	printf("Register 0x%02X = 0x%02X\n",register_address, val);

	if(print_logical) {
		printf("Motion Alert INT   %d\n", bma020_is_motion_alert_int());
		printf("Any Motion INT     %d\n", bma020_is_any_motion_int());
		printf("HG Counter         %d\n", bma020_get_counter_hg());
		printf("LG Counter         %d\n", bma020_get_counter_lg());
		printf("HG INT             %d\n", bma020_is_hg_int());
		printf("LG INT             %d\n", bma020_is_lg_int());
	}

}

/**
 * This method performs a simple read test
 *
 * After a  set of read values the
 * bma020 will set to sleep. while sleeping this method tries to read further and
 * should get zero-values. After that period the bma020 will wake up an a last
 * sequence of read outs will be performed.
 *
 * @param reset_bma020 - if true, resets the bma020, so that the defaults will be load
 * @param print_logical - if true, getters will executes and the results will be printed
 */
void test_bma020_read_and_sleep() {


	acceleration_t a_vector;
	uint8_t i;

	for(i = 0;i < 50;i++) {

		bma020_read_raw_acceleration(&a_vector);
		printf("X: %d Y: %d Z: %d\n",a_vector.x, a_vector.y, a_vector.z);

		_delay_ms(10);
	}

	printf("\n");
	printf("sleep well, little bma020..\n");
	printf("\n");

	bma020_sleep();

	for(i = 0;i < 50;i++) {

		bma020_read_raw_acceleration(&a_vector);
		printf("X: %d Y: %d Z:%d\n",a_vector.x, a_vector.y, a_vector.z);

		_delay_ms(10);
	}

	printf("\n");
	printf("wake up!\n");
	printf("\n");

	bma020_soft_reset();

	for(i = 0;i < 50;i++) {

		bma020_read_raw_acceleration(&a_vector);
		printf("X: %d Y: %d Z: %d\n",a_vector.x, a_vector.y, a_vector.z);

		_delay_ms(10);
	}
}
