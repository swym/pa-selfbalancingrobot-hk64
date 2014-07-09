/*
 * test_motionsensor.c
 *
 *  Created on: Jun 28, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_motionsensor.h"

/* * system headers              * */
#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

/* * local headers               * */
#include "../lib/uart.h"
#include "../lib/twi_master.h"
#include "../simplex-protocol.h"
#include "../lib/rfm12.h"

#include "../motionsensor.h"
#include "../mpu9150.h"

#include "../timer.h"

#include "../common.h"



/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
#define STATE_WAITING_FOR_USER_INTERRUPT_TIMEOUT	5			//Timeout in seconds
#define STATE_WAITING_FOR_USER_INTERRUPT_PARTS   	4

//Datatypes iterate higher nibble - lower nibble for sec-number
#define WIRELESS_TYPE_DATA_PID				0x80
#define WIRELESS_TYPE_DATA_ACCELERATION		0x90
#define WIRELESS_TYPE_DATA_ANGULARVELOCITY	0xA0
#define WIRELESS_SEND_BUFFER_MAX_LEN		10

/* * local objects               * */
static uint8_t wireless_send_buffer[WIRELESS_SEND_BUFFER_MAX_LEN];
static uint8_t wireless_send_buffer_len = 0;

static int16_t current_position;
static int16_t current_speed;

/* * local function declarations * */
static void test_motionsensor_init();
static inline void wireless_send_pid(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_motionsensor_init()
{
	DDR_LED = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */
	DDR_SCOPE = 0xFF;

	UART_init(56700);	/* Init UART mit 38400 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */

	timer_init();

	sei();

	simplex_protocol_init();
	rfm12_init();

	mpu9150_init();
	motionsensor_init();
	motionsensor_set_zero_point();
}

void test_motionsensor_run()
{
	test_motionsensor_init();

	printf("test_motionsensor_run\n");

	acceleration_t accel;

	uint8_t i;

	while(true) {

		if(timer_slot_1) {
			PORT_SCOPE ^= _BV(0);
			timer_slot_1 = false;

			current_position = motionsensor_get_position();
			mpu9150_read_acceleration(&accel);
			printf("% 4d%",i++);
			printf("% 5d% 5d% 5d",accel.x, accel.y, accel.z);
			printf("% 5d", mpu9150_read_angularvelocity_x());
			printf("% 5d", mpu9150_read_angularvelocity_y());
			printf("% 5d", mpu9150_read_angularvelocity_z());
			printf("%x",mpu9150_get_who_am_i());
			printf("\n");

			wireless_send_pid();

			PORT_SCOPE &= ~_BV(0);
		}
		simplex_protocol_tick();
	}
}


static inline void wireless_send_pid(void)
{
	//header
	wireless_send_buffer[0] = WIRELESS_TYPE_DATA_PID;
/*	uint32_t temp;


	//position
	temp = (uint32_t)(current_position);
	wireless_send_buffer[1] = (uint8_t)(temp >> 24);

	temp = (uint32_t)(current_position);
	temp = temp & 0x00FF0000;
	wireless_send_buffer[2] = (uint8_t)(temp >> 16);

	temp = (uint32_t)(current_position);
	temp = temp & 0x0000FF00;
	wireless_send_buffer[3] = (uint8_t)(temp >> 8);

	temp = (uint32_t)(current_position);
	wireless_send_buffer[4] = (uint8_t)((temp) & 0x000000FF);
*/
	//position
	wireless_send_buffer[1] = (uint8_t)(current_position >> 8);
	wireless_send_buffer[2] = (uint8_t)(current_position & 0x00FF);

	//speed
	wireless_send_buffer[3] = (uint8_t)((current_speed) >> 8);
	wireless_send_buffer[4] = (uint8_t)((current_speed) & 0x00FF);

	wireless_send_buffer_len = 5;

	simplex_protocol_send(SIMPLEX_PROTOCOL_FRAME_TYPE_DATA,
			wireless_send_buffer_len,
			wireless_send_buffer);
}


