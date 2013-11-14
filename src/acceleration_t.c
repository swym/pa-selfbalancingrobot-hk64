/*
 * acceleration_t.c
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************** */
#include "acceleration_t.h"

#include "bma020.h"
/* *** TODO BMA020
 * - Offset automatisch ermitteln
 * - evtl Werte glŠtten/filtern
 *
 *
 *
 * */



/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */
void acceleration_get_current_acceleration(acceleration_t *acceleration)
{
	acceleration_t offset;

	//TODO: Static offsets; should be measured!
//	offset.x = -3904;
//	offset.y =  1920;   //
//	offset.z =  1600;   //z:

	//read new value
	bma020_read_raw_acceleration(acceleration);

	//correct with offset
//	acceleration->x += offset.x;
//	acceleration->y += offset.y;
//	acceleration->z -= offset.z;
}
