/*
 * main.c
 *
 *  Created on: 04.09.2013
 *      Author: alexandermertens
 */

#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
	DDRC = 0xFF;

	uint8_t led = 1;

	for(;;) {

		PORTC = led;

		led++;
		_delay_ms(1000);
	}
}
