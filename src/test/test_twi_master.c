/*
 * test_twi_master.c
 *
 *  Created on: Nov 6, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "test_twi_master.h"

#include "../lib/twi_master.h"
/* *** DECLARATIONS ********************************************************** */


/* local type and constants     */

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */


void test_twi_master_get_bytes(void)
{
	uint8_t registers = 3;
	uint8_t register_addresses[3] = {0x15, 0x01, 0x00};
	uint8_t slave_address = (0x70 >> 1);
	uint8_t register_value;

	int i;
	for(i = 0;i < registers;i++) {
		register_value = twi_master_read_register(slave_address, register_addresses[i]);
		printf("Register 0x%02x = 0x%02x\n",register_addresses[i], register_value);
	}
}


void test_twi_master_write_and_read_bytes(void)
{

	uint8_t slave_address = (0x70 >> 1);

	uint8_t registers = 2;
	uint8_t register_addresses[2] = {0x12, 0x13};
	uint8_t register_values[2] = {0xAA, 0x55}; //0b1010 1010 und 0b0101 0101

	uint8_t val;
	uint8_t i;

	printf("read old values\n");

	for(i = 0;i < registers;i++) {
		val = twi_master_read_register(slave_address, register_addresses[i]);
		printf("Register 0x%02x = 0x%02x\n",register_addresses[i], val);
	}


	printf("write new register values\n");

	for(i = 0;i < registers;i++) {
		twi_master_write_register(slave_address,
								  register_addresses[i],
								  register_values[i]);
	}


	printf("read new values\n");
	for(i = 0;i < registers;i++) {
		val = twi_master_read_register(slave_address, register_addresses[i]);
		printf("Register 0x%02x = 0x%02x\n",register_addresses[i], val);
	}
}

void test_twi_master_read_and_write_bits(void)
{
	uint8_t slave_address = (0x70 >> 1);

	uint8_t registers = 2;
	uint8_t register_addresses[2] = {0x12, 0x13};
	//uint8_t register_values[2] = {0xAA, 0x55}; //0b1010 1010 und 0b0101 0101

	uint8_t val;
	uint8_t i;

	printf("wipe customer registers 1 and 2\n");

	for(i = 0;i < registers;i++) {
		twi_master_write_register(slave_address,
								  register_addresses[i], 0);
	}

	printf("write bits\n");

	for(i = 0;i < 8;i++) {
		twi_master_write_register_bit(slave_address,
									  register_addresses[0],i,true);

		val = twi_master_read_register(slave_address, register_addresses[0]);
		printf("Register 0x%02x = 0x%02x\n",register_addresses[0], val);
	}

}
