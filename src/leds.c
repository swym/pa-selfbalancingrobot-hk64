/*
 * leds.c
 *
 *  Created on: Feb 16, 2015
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "leds.h"

/* * system headers              * */
#include <avr/io.h>

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
#define	PORT_LEDS PORTA
#define DDR_LEDS  DDRA

#define PORT_RGB	PORTE
#define DDR_RGB		DDRE

#define LED_RED		PINE7
#define LED_BLUE	PINE6
#define LED_GREEN	PINE5


/* * local used ext. module objs * */

/* * local objects               * */
static uint8_t leds_value;

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */

void leds_inc(void)
{
	PORT_LEDS = leds_value++;
}

void leds_dec(void)
{
	PORT_LEDS = leds_value--;
}

void leds_set(uint8_t value)
{
	leds_value = value;
	PORT_LEDS = leds_value;
}

void leds_toggle(uint8_t value)
{
	leds_value ^= value;
	PORT_LEDS = leds_value;
}

void leds_color(leds_color_t color)
{
	PORT_RGB = color;
}

void leds_init(void)
{
	DDR_LEDS = 0xFF;
	DDR_RGB  = (_BV(LED_RED) | _BV(LED_BLUE) | _BV(LED_GREEN));
}
