/*
 * bma020.c
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************* */
#include "bma020.h"

/* system headers              */

/* local headers               */
#include "lib/twi_master.h"

/* *** DEFINES *** */
#define BMA020_TWI_ADRESS			(0x70>>1)


#define BMA020_VALUE_BANDWIDTH_0		0
#define BMA020_VALUE_BANDWIDTH_1		1
#define BMA020_VALUE_BANDWIDTH_2		2
#define BMA020_VALUE_RANGE_0			3
#define BMA020_VALUE_RANGE_1			4


#define BMA020_VALUE_LG_HYSTERESIS_0		0
#define BMA020_VALUE_LG_HYSTERESIS_1		1
#define BMA020_VALUE_LG_HYSTERESIS_2		2
#define BMA020_VALUE_HG_HYSTERESIS_0		3
#define BMA020_VALUE_HG_HYSTERESIS_1		4
#define BMA020_VALUE_HG_HYSTERESIS_2		5
#define BMA020_VALUE_ANY_MOTION_DURATION_0	6
#define BMA020_VALUE_ANY_MOTION_DURATION_1	7


#define BMA020_VALUE_WAKE_UP			0
#define BMA020_VALUE_WAKE_UP_PAUSE_0	1
#define BMA020_VALUE_WAKE_UP_PAUSE_1	2
#define BMA020_VALUE_SHADOW_DIS			3
#define BMA020_VALUE_LATCH_INT			4
#define BMA020_VALUE_NEW_DATA_INT		5
#define BMA020_VALUE_ADVANCED_INT		6
#define BMA020_VALUE_SPI4				7


#define BMA020_VALUE_ENABLE_LG			0
#define BMA020_VALUE_ENABLE_HG			1
#define BMA020_VALUE_COUNTER_LG_0		2
#define BMA020_VALUE_COUNTER_LG_1		3
#define BMA020_VALUE_COUNTER_HG_0		4
#define BMA020_VALUE_COUNTER_HG_1		5
#define	BMA020_VALUE_ANY_MOTION			6
#define BMA020_VALUE_ALTERT				7


#define	BMA020_VALUE_SLEEP				0
#define BMA020_VALUE_SOFT_RESET			1
#define	BMA020_VALUE_SELF_TEST_0		2
#define BMA020_VALUE_SELF_TEST_1		3
#define BMA020_VALUE_RESET_INT			6


#define BMA020_VALUE_STATUS_HG			0
#define BMA020_VALUE_STATUS_LG			1
#define BMA020_VALUE_HG_LATCHED			2
#define BMA020_VALUE_LG_LATCHED			3
#define BMA020_VALUE_ALERT_PHASE		4
#define BMA020_VALUE_SELF_TEST_RESULT	7




/* *** DECLARATIONS ********************************************************* */

/* local type and constants     */


/* local function declarations  */
static void bma020_set_register_bit(bool, uint8_t, uint8_t); /* bool enable, uint8_t register_adress, uint8_t bit */
static bool bma020_get_register_bit(uint8_t register_adress, uint8_t bit); /* uint8_t register_adress, uint8_t bit */

/**
 * These two bits (address 14h, bits 4 and 3) are used to select the full scale
 * acceleration range. Directly after changing the full scale range it takes
 * 1/(2*bandwidth) to overwrite the data registers with filtered data according
 * to the selected bandwidth.
 *
 * range<1:0>	Full scale acceleration range
 * 00			+/- 2g
 * 01			+/- 4g
 * 10			+/- 8g
 * 11			Not authorised code
 *
 * @param range; valid values: uint8_t = 2 || 4 || 8
 */
bool bma020_set_range(uint8_t range)
{
	if(range == 2 || range == 4 || range == 8) {

		uint8_t value;

		/* read register and delete old value */
		value = bma020_read_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
		value &= ~(1<<BMA020_VALUE_RANGE_1 | 1<<BMA020_VALUE_RANGE_0); /* (value &= 0b11100111;)*/


		if(range == 2) {
			/* if range == 2g; set no bits*/
		} else if(range == 4) {
			value |= (1<<BMA020_VALUE_RANGE_0);
		} else if(range == 8) {
			value |= (1<<BMA020_VALUE_RANGE_1);
		}

		bma020_write_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH, value);

		return true;
	} else {
		return false;
	}
}

uint8_t bma020_get_range(void)
{
	uint8_t register_value;
	uint8_t return_value;

	/* read register and delete all non range-bits*/
	register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
	register_value &= (1<<BMA020_VALUE_RANGE_1 | 1<<BMA020_VALUE_RANGE_0); /* (value &= 0b00011000;)*/

	/*convert to human readable values */
	if(register_value == 1<<BMA020_VALUE_RANGE_0) {
		return_value = 4;
	} else if (register_value == 1<<BMA020_VALUE_RANGE_1) {
		return_value = 8;
	} else {
		/* if no bit is set; range == 2 */
		return_value = 2;
	}

	return return_value;
}

/**
 * These three bits (address 14h, bits 2-0) are used to setup the digital
 * filtering of ADC output data to obtain the desired bandwidth. A second order
 * analogue filter defines the max. bandwidth to 1.5kHz. Digital filters can be
 * activated to reduce the bandwidth down to 25Hz in order to reduce signal
 * noise. The digital filters are moving average filters of various length with
 * a refresh rate of 3kHz. Since the bandwidth is reduced by a digital filter
 * for the factor 1/2 , 1/4, ... of the analogue filter frequency of 1.5kHz the
 * mean values of the bandwidth are slightly deviating from the rounded nominal
 * values. Table 4 shows the corresponding data:
 *
 * bandwidth<2:0>	Mean bandwidth[Hz]
 * 000				25
 * 001				50
 * 010				100
 * 011				190
 * 100				375
 * 101				750
 * 110				1500
 * 111				Not authorised code
 *
 * @param bandwidth
 */
bool bma020_set_bandwidth(uint16_t bandwidth)
{
	uint8_t value;

	if( bandwidth == 25   ||
		bandwidth == 50   ||
		bandwidth == 100  ||
		bandwidth == 190  ||
		bandwidth == 375  ||
		bandwidth == 750  ||
		bandwidth == 1500) {


		/* read register and delete old value */
		value = bma020_read_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
		value &= ~(1<<BMA020_VALUE_BANDWIDTH_2 |
				   1<<BMA020_VALUE_BANDWIDTH_1 |
				   1<<BMA020_VALUE_BANDWIDTH_0); /* value &= 0b11111000; */


		if(bandwidth == 25) {
			/* if bandwidth == 25; set no bits*/
		} else if(bandwidth == 50) {
			value |= (1<<BMA020_VALUE_BANDWIDTH_0);
		} else if(bandwidth == 100) {
			value |= (1<<BMA020_VALUE_BANDWIDTH_1);
		} else if(bandwidth == 190) {
			value |= (1<<BMA020_VALUE_BANDWIDTH_1 | 1<<BMA020_VALUE_BANDWIDTH_0);
		} else if(bandwidth == 375) {
			value |= (1<<BMA020_VALUE_BANDWIDTH_2);
		} else if(bandwidth == 750) {
			value |= (1<<BMA020_VALUE_BANDWIDTH_2 | 1<<BMA020_VALUE_BANDWIDTH_0);
		} else if(bandwidth == 1500) {
			value |= ((1<<BMA020_VALUE_BANDWIDTH_2) | (1<<BMA020_VALUE_BANDWIDTH_1));
		}

		bma020_write_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH, value);

		return true;
	} else {
		return false;
	}
}

uint16_t bma020_get_bandwidth(void)
{
	uint8_t register_value;
	uint16_t return_value;

	/* read register and delete all non range-bits*/
	register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
	register_value &= (1<<BMA020_VALUE_BANDWIDTH_2 |
					   1<<BMA020_VALUE_BANDWIDTH_1 |
					   1<<BMA020_VALUE_BANDWIDTH_0); /* (value &= 0b00000111;)*/

	/*convert to human readable values */
	if(register_value == 1<<BMA020_VALUE_BANDWIDTH_0) {
		return_value = 50;
	} else if(register_value ==  1<<BMA020_VALUE_BANDWIDTH_1)  {
		return_value = 100;
	} else if(register_value == (1<<BMA020_VALUE_BANDWIDTH_1 |
								 1<<BMA020_VALUE_BANDWIDTH_0)) {
		return_value = 190;
	} else if(register_value ==  1<<BMA020_VALUE_BANDWIDTH_2)  {
		return_value = 375;
	} else if(register_value == (1<<BMA020_VALUE_BANDWIDTH_2 |
								 1<<BMA020_VALUE_BANDWIDTH_0)) {
		return_value = 750;
	} else if(register_value == (1<<BMA020_VALUE_BANDWIDTH_2 |
								 1<<BMA020_VALUE_BANDWIDTH_1)) {
		return_value = 1500;
	} else {
		/* if no bit is set; bandwidth == 25*/
		return_value = 25;
	}

	return return_value;
}

/**
 * These bits (address 15h, bit 2 and 1) define the sleep phase duration between
 * each automatic wake-up.
 * wake_up_pause<1:0>	Sleep phase duration
 * 00					20 ms
 * 01					80 ms
 * 10					320 ms
 * 11					2560 ms
 *
 * @param wake_up_time
 * @return
 */

bool bma020_set_wake_up_pause(uint16_t wake_up_time)
{
	uint8_t register_value;

	if( wake_up_time == 20   ||
		wake_up_time == 80   ||
		wake_up_time == 320  ||
		wake_up_time == 2560) {

		/* read register and delete old value */
		register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_INT);
		register_value &= ~(1<<BMA020_VALUE_WAKE_UP_PAUSE_1 |
							1<<BMA020_VALUE_WAKE_UP_PAUSE_0); /* value &= 0b11111001; */


		if(wake_up_time == 20) {
			/* set no bits */
		} else if (wake_up_time == 80) {
			register_value |= (1<<BMA020_VALUE_WAKE_UP_PAUSE_0);
		} else if(wake_up_time == 320) {
			register_value |= (1<<BMA020_VALUE_WAKE_UP_PAUSE_1);
		} else if(wake_up_time == 2560) {
			register_value |= (1<<BMA020_VALUE_WAKE_UP_PAUSE_0 |
							   1<<BMA020_VALUE_WAKE_UP_PAUSE_1);
		}

		bma020_write_register_value(BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH,
									register_value);

		return true;
	} else {
		return false;
	}
}

uint16_t bma020_get_wake_up_pause(void)
{
	uint8_t register_value;
	uint16_t return_value;


	/* read register and delete all non wake_up_pause */
	register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_INT);
	register_value &= (1<<BMA020_VALUE_WAKE_UP_PAUSE_1 |
					   1<<BMA020_VALUE_WAKE_UP_PAUSE_0);

	/* convert to human readable values*/
	if(register_value == 1<<BMA020_VALUE_WAKE_UP_PAUSE_0) {
		return_value = 80;
	} else if(register_value ==  1<<BMA020_VALUE_WAKE_UP_PAUSE_1) {
		return_value = 320;
	} else if(register_value == (1<<BMA020_VALUE_WAKE_UP_PAUSE_0 |
								 1<<BMA020_VALUE_WAKE_UP_PAUSE_1)) {
		return_value = 2560;
	} else {
		return_value = 20;
	}

	return return_value;
}


bool bma020_set_counter_lg(uint8_t counter)
{
	if(counter == 0  || counter == 1 || counter == 2 || counter == 3) {

		uint8_t register_value;

		/* read register and delete old value */
		register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE);
		register_value &= ~(1<<BMA020_VALUE_COUNTER_LG_1 |
							1<<BMA020_VALUE_COUNTER_LG_0); /* (value &= 0b11110011;)*/

		/* set new value (shift counter value on correct position */
		register_value |= counter<<BMA020_VALUE_COUNTER_LG_0;

		/*write new value to register */
		bma020_write_register_value(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
									register_value);

		return true;
	} else {
		return false;
	}
}



uint8_t bma020_get_counter_lg(void)
{
	uint8_t register_value;

	/* read register and delete all non counter lg bits */
	register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE);
	register_value &= (1<<BMA020_VALUE_COUNTER_LG_1 |
					   1<<BMA020_VALUE_COUNTER_LG_0);

	return (register_value >> BMA020_VALUE_COUNTER_LG_0);
}


bool bma020_set_counter_hg(uint8_t counter)
{
	if(counter == 0  || counter == 1 || counter == 2 || counter == 3) {

		uint8_t register_value;

		/* read register and delete old value */
		register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE);
		register_value &= ~(1<<BMA020_VALUE_COUNTER_HG_1 |
							1<<BMA020_VALUE_COUNTER_HG_0); /* (value &= 0b11110011;)*/

		/* set new value (shift counter value on correct position */
		register_value |= counter<<BMA020_VALUE_COUNTER_HG_0;

		/*write new value to register */
		bma020_write_register_value(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
									register_value);

		return true;
	} else {
		return false;
	}
}

uint8_t bma020_get_counter_hg(void)
{
	uint8_t register_value;

	/* read register and delete all non counter lg bits */
	register_value = bma020_read_register_value(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE);
	register_value &= (1<<BMA020_VALUE_COUNTER_HG_1 |
					   1<<BMA020_VALUE_COUNTER_HG_0);

	return (register_value >> BMA020_VALUE_COUNTER_HG_0);
}

//Any_motion_dur is used to filter the motion profile and also to define a minimum interrupt duration because the reset condition is also filtered.
bool bma020_set_any_motion_duration(uint8_t duration)
{
	uint8_t register_value;

	if(duration == 1 || duration == 3 || duration == 5 || duration == 7) {


		/* read register and delete old value */
		register_value = bma020_read_register_value(BMA020_REGISTER_AMD_HGH_LGH);
		register_value &= ~(1<<BMA020_VALUE_ANY_MOTION_DURATION_1 |
							1<<BMA020_VALUE_ANY_MOTION_DURATION_0); /* value &= 0b00111111; */


		if(duration == 1) {
			/* set no bits*/
		} else if(duration == 3) {
			register_value |= 1<<BMA020_VALUE_ANY_MOTION_DURATION_0;
		} else if(duration == 5) {
			register_value |= 1<<BMA020_VALUE_ANY_MOTION_DURATION_1;
		} else if(duration == 7) {
			register_value |= (1<<BMA020_VALUE_ANY_MOTION_DURATION_0 |
							   1<<BMA020_VALUE_ANY_MOTION_DURATION_1);
		}

		bma020_write_register_value(BMA020_REGISTER_AMD_HGH_LGH,
									register_value);

		return true;
	} else {
		return false;
	}
}

uint8_t bma020_get_any_motion_duration(void)
{
	uint8_t register_value;
	uint16_t return_value;

	/* read register and delete all non any_motion_duration bits */
	register_value = bma020_read_register_value(BMA020_REGISTER_AMD_HGH_LGH);
	register_value &= (1<<BMA020_VALUE_ANY_MOTION_DURATION_0 |
					   1<<BMA020_VALUE_ANY_MOTION_DURATION_1);

	/* convert to human readable values*/
	if(register_value == 1<<BMA020_VALUE_ANY_MOTION_DURATION_0) {
		return_value = 3;
	} else if(register_value == 1<<BMA020_VALUE_ANY_MOTION_DURATION_0) {
		return_value = 5;
	} else if(register_value == (1<<BMA020_VALUE_ANY_MOTION_DURATION_0 |
								 1<<BMA020_VALUE_ANY_MOTION_DURATION_1)) {
		return_value = 7;
	} else {
		return_value = 1;
	}

	return return_value;
}


void bma020_set_lg_hysterese(uint8_t hysterese)
{
	uint8_t register_value;

	/* only the lowest three bits are valid */
	hysterese &= 0x07;

	/* read register and delete old value */
	register_value  = bma020_read_register_value(BMA020_REGISTER_AMD_HGH_LGH);
	register_value &= ~(BMA020_VALUE_LG_HYSTERESIS_0 |
						BMA020_VALUE_LG_HYSTERESIS_1 |
						BMA020_VALUE_LG_HYSTERESIS_2); /* value &= 0b00000111; */

	/* set new value */
	register_value |= hysterese;

	/*write back to register */
	bma020_write_register_value(BMA020_REGISTER_AMD_HGH_LGH,
								register_value);
}

uint8_t bma020_get_lg_hysterese(void)
{
	uint8_t register_value;

	register_value = bma020_read_register_value(BMA020_REGISTER_AMD_HGH_LGH);

	/* only the lowest three bits represents the hyterese */
	return (register_value & 0x07);
}


void bma020_set_hg_hysterese(uint8_t hysterese)
{
	uint8_t register_value;

	/* only the lowest three bits are valid */
	hysterese &= 0x07;

	/* read register and delete old value */
	register_value  = bma020_read_register_value(BMA020_REGISTER_AMD_HGH_LGH);
	register_value &= ~(BMA020_VALUE_HG_HYSTERESIS_0 |
						BMA020_VALUE_HG_HYSTERESIS_1 |
						BMA020_VALUE_HG_HYSTERESIS_2); /* value &= 0b00111000; */

	/* set new value */
	register_value |= hysterese<<BMA020_VALUE_HG_HYSTERESIS_0;

	/*write back to register */
	bma020_write_register_value(BMA020_REGISTER_AMD_HGH_LGH,
								register_value);
}


uint8_t bma020_get_hg_hysterese(void)
{
	uint8_t register_value;

	register_value = bma020_read_register_value(BMA020_REGISTER_AMD_HGH_LGH);

	/* only the bits 0b00111000 are the hyterese */
	register_value &= 0x38;
	register_value = register_value>>3;
	return register_value;
}


void bma020_set_any_motion_threshold(uint8_t threshold)
{
	bma020_write_register_value(BMA020_REGISTER_ANY_MOTION_THRESHOLD,
								threshold);
}

uint8_t bma020_get_any_motion_threshold(void)
{
	return bma020_read_register_value(BMA020_REGISTER_ANY_MOTION_THRESHOLD);
}


void bma020_set_hg_duration(uint8_t duration)
{
	bma020_write_register_value(BMA020_REGISTER_HG_DURATION,
								duration);
}

uint8_t bma020_get_hg_duration()
{
	return bma020_read_register_value(BMA020_REGISTER_HG_DURATION);
}


void bma020_set_lg_duration(uint8_t duration)
{
	bma020_write_register_value(BMA020_REGISTER_LG_DURATION,
								duration);
}

uint8_t bma020_get_lg_duration()
{
	return bma020_read_register_value(BMA020_REGISTER_LG_DURATION);
}

void bma020_set_customer_reserved_1(uint8_t duration)
{
	bma020_write_register_value(BMA020_REGISTER_CONTROL_CUSTOMER_1,
								duration);
}

uint8_t bma020_get_customer_reserved_1()
{
	return bma020_read_register_value(BMA020_REGISTER_CONTROL_CUSTOMER_1);
}

void bma020_set_customer_reserved_2(uint8_t duration)
{
	bma020_write_register_value(BMA020_REGISTER_CONTROL_CUSTOMER_2,
								duration);
}

uint8_t bma020_get_customer_reserved_2()
{
	return bma020_read_register_value(BMA020_REGISTER_CONTROL_CUSTOMER_2);
}

void bma020_set_enable_lg(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
							BMA020_VALUE_ENABLE_LG);
}

bool bma020_get_enable_lg(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
								   BMA020_VALUE_ENABLE_LG);
}



void bma020_set_enable_hg(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
							BMA020_VALUE_ENABLE_HG);
}

bool bma020_get_enable_hg(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
								   BMA020_VALUE_ENABLE_HG);
}




void bma020_set_enable_any_motion(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
							BMA020_VALUE_ANY_MOTION);
}

bool bma020_get_enable_any_motion(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
								   BMA020_VALUE_ANY_MOTION);
}




void bma020_set_enable_motion_alert(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
							BMA020_VALUE_ALTERT);
}

bool bma020_get_enable_motion_alert(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_COUNTERS_ALERT_HG_LG_ENABLE,
								   BMA020_VALUE_ALTERT);
}




void bma020_set_new_data_int(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_INT,
							BMA020_VALUE_NEW_DATA_INT);
}

bool bma020_get_new_data_int(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_INT,
								   BMA020_VALUE_NEW_DATA_INT);
}

void bma020_set_wake_up(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_INT,
							BMA020_VALUE_WAKE_UP);
}

bool bma020_get_wake_up(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_INT,
								   BMA020_VALUE_WAKE_UP);
}


void bma020_set_shadow_dis(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_INT,
							BMA020_VALUE_SHADOW_DIS);
}

bool bma020_get_shadow_dis(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_INT,
								   BMA020_VALUE_SHADOW_DIS);
}

void bma020_set_latched_int(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_INT,
							BMA020_VALUE_LATCH_INT);
}

bool bma020_get_latched_int(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_INT,
								   BMA020_VALUE_LATCH_INT);
}


void bma020_set_advanced_int(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_INT,
							BMA020_VALUE_ADVANCED_INT);
}

bool bma020_get_advanced_int(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_INT,
								   BMA020_VALUE_ADVANCED_INT);
}

void bma020_set_spi4(bool enable)
{
	bma020_set_register_bit(enable,
							BMA020_REGISTER_CONTROL_INT,
							BMA020_VALUE_SPI4);
}

bool bma020_get_spi4(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_CONTROL_INT,
								   BMA020_VALUE_SPI4);
}


void bma020_reset_interrupt(void)
{
	bma020_set_register_bit(true,
							BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
							BMA020_VALUE_RESET_INT);
}

void bma020_perform_self_test_0(void)
{
	bma020_set_register_bit(true,
							BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
							BMA020_VALUE_SELF_TEST_0);
}

void bma020_perform_self_test_1(void)
{
	bma020_set_register_bit(true,
							BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
							BMA020_VALUE_SELF_TEST_1);
}

void bma020_soft_reset(void)
{
	bma020_set_register_bit(true,
							BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
							BMA020_VALUE_SOFT_RESET);
}


void bma020_sleep(void)
{
	bma020_set_register_bit(true,
							BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
							BMA020_VALUE_SLEEP);
}
bool bma020_get_self_test_result(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_STATUS,
								   BMA020_VALUE_SELF_TEST_RESULT);
}



bool bma020_get_status_hg(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_STATUS,
								   BMA020_VALUE_STATUS_HG);
}



bool bma020_get_status_lg(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_STATUS,
								   BMA020_VALUE_STATUS_LG);
}


bool bma020_get_hg_latched(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_STATUS,
								   BMA020_VALUE_HG_LATCHED);
}

void bma020_reset_hg_latched(void)
{
	bma020_set_register_bit(false,
							BMA020_REGISTER_STATUS,
							BMA020_VALUE_HG_LATCHED);
}

bool bma020_get_lg_latched(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_STATUS,
								   BMA020_VALUE_LG_LATCHED);
}

void bma020_reset_lg_latched(void)
{
	bma020_set_register_bit(false,
							BMA020_REGISTER_STATUS,
							BMA020_VALUE_LG_LATCHED);
}


bool bma020_get_alert_phase(void)
{
	return bma020_get_register_bit(BMA020_REGISTER_STATUS,
								   BMA020_VALUE_ALERT_PHASE);
}

void bma020_reset_alert_phase(void)
{
	bma020_set_register_bit(false,
							BMA020_REGISTER_STATUS,
							BMA020_VALUE_ALERT_PHASE);
}

bool bma020_new_data(char axis)
{
	uint8_t register_adress;

	if(axis == 'x') {
		register_adress = BMA020_REGISTER_VALUE_X_LSB;
	} else if(axis == 'y') {
		register_adress = BMA020_REGISTER_VALUE_Y_LSB;
	} else if(axis == 'z') {
		register_adress = BMA020_REGISTER_VALUE_Z_LSB;
	} else {
		return false;
	}

	return bma020_get_register_bit(register_adress, 0);
}

void bma020_set_register_bit(bool enable, uint8_t register_adress, uint8_t bit)
{
	uint8_t register_value;

	/* read register and delete data_int*/
	register_value = bma020_read_register_value(register_adress);
	register_value &= ~(1<<bit);

	if(enable) {
		register_value |= (1<<bit);
	}

	//set register
	bma020_write_register_value(register_adress, register_value);
}

uint8_t bma020_get_ml_version()
{
	uint8_t register_value;

	register_value = bma020_read_register_value(BMA020_REGISTER_VERSION);

	return (0x0F & register_value);
}


uint8_t bma020_get_ai_version()
{
	uint8_t register_value;
	register_value = bma020_read_register_value(BMA020_REGISTER_VERSION);
	return (register_value >> 4);
}

uint8_t bma020_get_chip_id()
{
	uint8_t register_value;
	register_value = bma020_read_register_value(BMA020_REGISTER_VERSION);
	return (0x07 & register_value);
}


bool bma020_get_register_bit(uint8_t register_adress, uint8_t bit)
{
	uint8_t register_value;

	register_value = bma020_read_register_value(register_adress);

	if(register_value & (1<<bit)) {
		return true;
	} else {
		return false;
	}
}




uint8_t bma020_read_register_value(uint8_t adress)
{
	uint8_t value;

	twi_send_buffer[0] = adress;

	twi_master_set_ready();
	twi_send_data(BMA020_TWI_ADRESS, 1);
	twi_receive_data(BMA020_TWI_ADRESS, 1);

	value = twi_receive_buffer[0];

	return value;
}


void bma020_write_register_value(uint8_t adress, uint8_t value)
{
	twi_send_buffer[0] = adress;
	twi_send_buffer[1] = value;

	twi_master_set_ready();
	twi_send_data(BMA020_TWI_ADRESS, 2);
}




/* *** */
static bma020_t self_;


/* *** FUNCTION DEFINITIONS ************************************************* */
void bma020_init(void)
{
	self_.range = 2;
	self_.bandwidth = 25;
}

int16_t bma020_read_raw_z()
{
	uint8_t z_lsb = 0;
	uint8_t z_msb = 0;

	int16_t z = 0;

	/* read register */
	twi_send_buffer[0] = BMA020_REGISTER_VALUE_Z_LSB;

	twi_master_set_ready();
	twi_send_data(BMA020_TWI_ADRESS, 1);
	twi_receive_data(BMA020_TWI_ADRESS, 2);

	/* convert value */
	z_lsb = twi_receive_buffer[0];
	z_msb = twi_receive_buffer[1];

	z = (z_msb << 2) | (z_lsb >> 6);

	//if z is negative correct minus sign
	if(z & 0x0200) {
		z |= 0xfc00;
	}

	return z;
}

void bma020_read_raw_acceleration(acceleration_t* raw_vector)
{
	uint8_t temp_data;

	twi_send_buffer[0] = BMA020_REGISTER_VALUE_X_LSB;

	twi_master_set_ready();
	twi_send_data(BMA020_TWI_ADRESS, 1);
	twi_receive_data(BMA020_TWI_ADRESS, 6);

	temp_data = twi_receive_buffer[0];
	raw_vector->x = (uint16_t)((temp_data & 0xC0));
	temp_data = twi_receive_buffer[1];
	raw_vector->x = (uint16_t)(raw_vector->x | temp_data << 8);

	temp_data = twi_receive_buffer[2];
	raw_vector->y = (uint16_t)((temp_data & 0xC0));
	temp_data = twi_receive_buffer[3];
	raw_vector->y = (uint16_t)(raw_vector->y | temp_data << 8);

	temp_data = twi_receive_buffer[4];
	raw_vector->z = (uint16_t)((temp_data & 0xC0));
	temp_data = twi_receive_buffer[5];
	raw_vector->z = (uint16_t)(raw_vector->z | temp_data << 8);
}

