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
#include <stdint.h>
#include <stdbool.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define TIMER_SLOT_COUNTER_MAX		4 // (Intervalll�nge = 20 ms / IRQ-Zeit = 4 ms)
#define TIMER_TWI_READY_TIMEOUT_MS	2

#define TIMER_MAJORSLOT_NONE		0
#define TIMER_MAJORSLOT_0			1
#define TIMER_MAJORSLOT_1			2

#define TIMER_MINORSLOT_NONE		0
#define TIMER_MINORSLOT_0			1

typedef uint8_t timer_slot_t;

/* * external objects            * */
extern volatile timer_slot_t timer_current_majorslot;
extern volatile timer_slot_t timer_current_minorslot;

extern volatile uint8_t timer_twi_ready_timeout;

/* * external functions          * */
extern void timer_init(void);

#endif /* TIMER_H_ */
