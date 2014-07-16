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
#include "system_controller.h"

/* Tests */
#include "test/test_filters.h"


/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

/* local function declarations  */
static void main_run(void);
static void main_run_tests(void);

/* *** FUNCTION DEFINITIONS ************************************************** */

void main_run(void)
{
	system_controller_state_machine();
}

void main_run_tests(void)
{
	test_filters_run();
}

/**
 * @details Does something important
 * @return
 */
int main(void)
{
//	main_run_tests();

	main_run();

	return 0;
}



