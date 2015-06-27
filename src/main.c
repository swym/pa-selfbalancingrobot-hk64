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

#include <stdio.h>
#include <stdbool.h>

/* local headers               */
//#include "system_controller.h"

/* Tests */
#include "test/test_filters.h"
/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */

/* *** FUNCTION DEFINITIONS ************************************************** */

/**
 * @details Does something important
 * @return
 */
int main(void)
{
	// main program
	//system_controller_state_machine();
	test_filters_run();

	//never reached
	return 0;
}



