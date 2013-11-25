/*
 * moving_average.c
 *
 *  Created on: Nov 14, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "moving_average.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */

void moving_average_simple_put_element(moving_average_t *average, int16_t value)
{
	average->elements_sum -= average->elements[average->index];	//zu UEberschreibenen Wert aus der Summe loeschen
	average->elements_sum += value;							//Neuen Wert der Summe hinzufuegen

	average->elements[average->index] = value;		//Neuen Wert im Array eintragen.

	//Neuen Mittelwert berrechnen
	average->mean = average->elements_sum / MOVING_AVERAGE_ELEMENT_COUNT;

	//Index auf die neue Stelle schieben
	if(average->index < MOVING_AVERAGE_ELEMENT_COUNT - 1) {
		average->index++;
	} else {
		average->index = 0;
	}
}
