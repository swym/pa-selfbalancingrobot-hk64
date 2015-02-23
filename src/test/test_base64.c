/*
 * test_base64.c
 *
 *  Created on: Feb 21, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "test_base64.h"

/* * system headers              * */
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* * local headers               * */
#include "../base64.h"

#include "../lib/uart.h"
#include "../leds.h"


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local used ext. module objs * */
uint8_t base64_encode_buffer[BASE64_BUFFER_SIZE];

/* * local objects               * */

/* * local function declarations * */
void test_base64_init(void);


/* *** FUNCTION DEFINITIONS ************************************************* */
void test_base64_run(void)
{
	test_base64_init();

	uint8_t data[] = {'4', ':', 0x00, 0x00, 0xFF,0xFF};
	uint8_t data_size = 6;

	uint8_t enc_bytes;

	for(;;) {

		enc_bytes = base64_encode(data, data_size);

		if(enc_bytes) {
			printf("%s\n", base64_encode_buffer);
			printf("enc_bytes: %d\n", enc_bytes);
		} else {
			printf("ERROR - ENC BUFFER TO SMALL\n");
		}

		leds_inc();

		_delay_ms(1000);
	}
}


void test_base64_init(void)
{

	UART_init();	/* Init UART */
	leds_init();

	printf("init\n");
}
