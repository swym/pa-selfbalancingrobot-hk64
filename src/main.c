/*
 * main.c
 *
 *  Created on: 04.09.2013
 *      Author: alexandermertens
 */

#include <avr/io.h>


int main(void)
{
	DDRC = 0xFF;

	while(1) {
		PORTC = 0b01010101;
	}
}
