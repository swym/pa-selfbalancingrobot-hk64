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


#define BMA020_VALUE_BANDWIDTH_0	0
#define BMA020_VALUE_BANDWIDTH_1	1
#define BMA020_VALUE_BANDWIDTH_2	2

#define BMA020_VALUE_RANGE_0		3
#define BMA020_VALUE_RANGE_1		4




/* *** DECLARATIONS ********************************************************* */

/* local type and constants     */


/* local function declarations  */

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

	if(register_value == 1<<BMA020_VALUE_BANDWIDTH_0) {
		return_value = 50;
	} else if(register_value == 1<<BMA020_VALUE_BANDWIDTH_1) {
		return_value = 100;
	} else if(register_value == (1<<BMA020_VALUE_BANDWIDTH_1 |
								 1<<BMA020_VALUE_BANDWIDTH_0)) {
		return_value = 190;
	} else if(register_value == 1<<BMA020_VALUE_BANDWIDTH_2) {
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

