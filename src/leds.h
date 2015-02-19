/*
 * leds.h
 *
 *  Created on: Feb 16, 2015
 *      Author: alexandermertens
 */

#ifndef LEDS_H_
#define LEDS_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <avr/io.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */

#define	PORT_LEDS PORTA
#define DDR_LEDS  DDRA

#define LED0	PINA0
#define LED1	PINA1
#define LED2	PINA2
#define LED3	PINA3
#define LED4	PINA4
#define LED5	PINA5
#define LED6	PINA6
#define LED7	PINA7

#define LEDS_VALUE_MAX			0x07

/* * external objects            * */

/* * external functions          * */
extern void leds_inc();
extern void leds_init(void);

#endif /* LEDS_H_ */
