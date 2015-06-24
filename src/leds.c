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

/* * local used ext. module objs * */

/* * local objects               * */
static uint8_t leds_value;

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */

void leds_inc(void)
{
	PORT_LEDS = leds_value++;
}

void leds_init(void)
{
	DDR_LEDS = 0xFF;
}
