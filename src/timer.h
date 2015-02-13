/*
 * timer.h
 *
 *  Created on: Feb 13, 2015
 *      Author: alexandermertens
 */

#ifndef TIMER_H_
#define TIMER_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdlib.h>
#include <stdbool.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define TIMER_SLOT_COUNTER_MAX		4 // (Intervalll�nge = 20 ms / IRQ-Zeit = 4 ms)
#define TIMER_TWI_READY_TIMEOUT_MS	2

/* * external objects            * */
extern volatile bool timer_slot_0;
extern volatile bool timer_slot_1;
extern volatile bool timer_slot_2;
extern volatile bool timer_slot_3;
extern volatile uint8_t timer_twi_ready_timeout;

/* * external functions          * */
extern void timer_init(void);

#endif /* TIMER_H_ */
