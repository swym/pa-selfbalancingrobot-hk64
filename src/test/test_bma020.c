/*
 * test_functionality.c
 *
 *  Created on: Nov 6, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "test_bma020.h"
/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void test_bma020_set_range(void);
static void test_bma020_set_bandwidth(void);
static void test_bma020_int(void);
static void test_bma020(void);

/* *** FUNCTION DEFINITIONS ************************************************** */



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
