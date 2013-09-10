/*
 * main.c
 *
 *  Created on: 04.09.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
/* modul header                */
#include "main.h"

/* system headers              */
#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdbool.h>

/* local headers               */
#include "lib/uart.h"
#include "lib/twi_master.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void main_init(void);
static void main_run(void);
static void test_twi_with_bma020(void);

/* *** FUNCTION DEFINITIONS ************************************************** */

void main_init(void)
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definierten */

	UART_init(9600);	/* Init UART mit 9600 baud */

	twi_master_init();		/* Init TWI/I2C Schnittstelle */

	sei();				/* Enable global interrupts */
}


void main_run(void)
{
	uint8_t led = 1;

	for(;;) {

		PORTC = led;

		printf("LED: %d\n", led);
		test_twi_with_bma020();

		led++;
		_delay_ms(50);
	}
}

void test_twi_with_bma020(void)
{

	const uint8_t BMA020_ADRESS = (0x70 >> 1);
	const uint8_t BMA020_X_LSB  = 0x02;

	uint8_t temp_data;
	uint16_t x, y, z;


	twi_send_buffer[0] = BMA020_X_LSB;

	twi_master_set_ready();
	twi_send_data(BMA020_ADRESS, 1);
	twi_receive_data(BMA020_ADRESS, 6);

	temp_data = twi_receive_buffer[0];
	x = (uint16_t)(temp_data << 8);

	temp_data = twi_receive_buffer[1];
	x = (uint16_t)(x | (temp_data & 0xC0));

	temp_data = twi_receive_buffer[2];
	y = (uint16_t)(temp_data << 8);

	temp_data = twi_receive_buffer[3];
	y = (uint16_t)(y | (temp_data & 0xC0));

	temp_data = twi_receive_buffer[4];
	z = (uint16_t)(temp_data << 8);

	temp_data = twi_receive_buffer[5];
	z = (uint16_t)(z | (temp_data & 0xC0));


	printf("X:%i Y:%i Z:%i\n",x, y, z);
}


int main(void)
{
	main_init();
	main_run();
}



