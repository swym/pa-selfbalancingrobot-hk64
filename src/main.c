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
//#include "test/test_encoder.h"
//#include "test/test_plot_data.h"
//#include "test/test_command_parser.h"
#include "test/test_serialport.h"

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

	//test_encoder_run();
	//test_data_plot();

	//system_controller_state_machine();
	//test_command_parser();
	test_serialport();

	//never reached
	return 0;
}



