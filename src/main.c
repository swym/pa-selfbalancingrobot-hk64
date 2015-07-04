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
#include <stdio.h>
#include <stdbool.h>

/* local headers               */
#include "system_controller.h"

/* Tests */

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */

int main(void)
{

	// main program
	system_controller_state_machine();

	//never reached
	return 0;
}



