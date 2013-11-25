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

#include "acceleration_t.h"
#include "motor_control.h"
#include "bma020.h"
#include "controller.h"

/* Tests */
//#include "test/test_twi_master.h"
//#include "test/test_bma020.h"
#include "test/test_motor_control.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void main_init(void);
static void main_run(void);
static void main_run_tests(void);

/* *** FUNCTION DEFINITIONS ************************************************** */

void main_init(void)
{
	DDRC = 0xFF;		/* Data Direction Register der LEDs als Ausgang definieren */

	UART_init(9600);	/* Init UART mit 9600 baud */
	twi_master_init();	/* Init TWI/I2C Schnittstelle */
	sei();				/* Enable global interrupts */

	acceleration_init();
	motor_control_init();

	controller_init();


}


void main_run(void)
{
	controller_run();
/*
	uint8_t led = 0;


	for(;;) {
		PORTC = led;
		led++;

		_delay_ms(500);
	}
*/
}


void main_run_tests(void)
{
	//test_twi_master_get_bytes();
	//test_master_write_and_read_bytes();
	//test_twi_master_read_and_write_bits();

//	uint8_t i;
//	uint8_t max = 1;

//	for(i = 0;i < max;i ++) {
//		test_bma020_settings();
//		_delay_ms(100);
//	}
	//test_motor_control_set_different_speeds();


}


/**
 * @details Does something important
 * @return
 */
int main(void)
{
	main_init();

	main_run_tests();
	main_run();

	return 0;
}



