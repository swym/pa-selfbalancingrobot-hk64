/*
 * test_functionality.c
 *
 *  Created on: Nov 6, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "test_bma020.h"

#include <util/delay.h>
/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void test_bma020_register0x15(bool, bool);
static void test_bma020_register0x14(bool, bool);
static void test_bma020_register0x11(bool, bool);
static void test_bma020_register0x0B(bool, bool);

/* *** FUNCTION DEFINITIONS ************************************************** */


void test_bma020_settings()
{
//	test_bma020_register0x15(true, true);
//	test_bma020_register0x14(true, true);
//	test_bma020_register0x11(false, true);
	test_bma020_register0x0B(true, true);


}


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

/* OLD AND BUSTED ************************************************************ */

void test_bma020_int(void)
{
/*	acceleration_t a_vector;

	bma020_read_raw_acceleration(&a_vector);
	printf("Z:%i\n",a_vector.z);

*/

/*
	uint16_t delay = 1000;

	bma020_set_new_data_int(true);
	printf("register_value is: 0x%X ", bma020_read_register_value(BMA020_REGISTER_CONTROL_INT));
	printf("New Data INT is: %d\n",bma020_get_new_data_int());

	_delay_ms(delay);

	bma020_set_new_data_int(false);
	printf("register_value is: 0x%X ", bma020_read_register_value(BMA020_REGISTER_CONTROL_INT));
	printf("New Data INT is: %d\n",bma020_get_new_data_int());

	_delay_ms(delay);
*/
}

void test_bma020_set_bandwidth(void)
{
	/*
	uint16_t values[9] = {25,50,100,190,375,750,1500,10,13};
	uint8_t values_count = 9;
	uint16_t delay = 1000;

	bool print_register = true;
	bool return_val;

	int i;
	for(i = 0; i < values_count;i++) {
		return_val = bma020_set_bandwidth(values[i]);

		printf("try to set bandwidth to %i  ", values[i]);

		if(return_val) {
			printf("succeded!  ");
		} else {
			printf("failed!    ");
		}

		if(print_register) {
			printf("register_value is: 0x%x ", bma020_read_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH));
			printf("bandwidth is: %d ", bma020_get_bandwidth());
		}

		printf("\n");

		_delay_ms(delay);
	}
	*/
}

void test_bma020_set_range(void)
{
	/*
	uint16_t values[5] = {2,4,8,6,10};
	uint8_t values_count = 5;
	uint16_t delay = 1000;

	bool print_register = true;
	bool return_val;

	int i;
	for(i = 0; i < values_count;i++) {
		return_val = bma020_set_range(values[i]);

		printf("try to set range to %i  ", values[i]);

		if(return_val) {
			printf("succeded!  ");
		} else {
			printf("failed!    ");
		}

		if(print_register) {
			printf("register_value is: 0x%x ", bma020_read_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH));
			printf("range is: %d ", bma020_get_range());
		}

		printf("\n");

		_delay_ms(delay);
	}
	*/
}

void test_bma020(void)
{
	/*
	acceleration_t a_vector;

	bma020_read_raw_acceleration(&a_vector);

	printf("Z:%i\n",a_vector.z);

//	printf("X:%i Y:%i Z:%i\n",a_vector.x, a_vector.y, a_vector.z);
 * */

}
