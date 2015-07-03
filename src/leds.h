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


/* * external objects            * */
#define LED0	PINA0
#define LED1	PINA1
#define LED2	PINA2
#define LED3	PINA3
#define LED4	PINA4
#define LED5	PINA5
#define LED6	PINA6
#define LED7	PINA7


typedef enum {
	LEDS_COLOR_BLACK	= 0x00,
	LEDS_COLOR_WHITE	= 0xE0,

	LEDS_COLOR_RED		= 0x80,
	LEDS_COLOR_GREEN	= 0x20,
	LEDS_COLOR_BLUE 	= 0x40,

	LEDS_COLOR_CYAN 	= 0x60,
	LEDS_COLOR_MAGENTA	= 0xC0,
	LEDS_COLOR_YELLOW	= 0xA0
} leds_color_t;

/* * external functions          * */
extern void leds_inc();
extern void leds_dec();
extern void leds_set(uint8_t value);
extern void leds_toggle(uint8_t value);
extern void leds_color(leds_color_t color);
extern void leds_init(void);

#endif /* LEDS_H_ */
