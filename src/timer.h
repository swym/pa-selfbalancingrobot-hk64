/*
 * timer.h
 *
 *  Created on: 25.11.2013
 *      Author: alexandermertens
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdbool.h>
#include <stdint.h>

#define TIMER_SLOT_COUNTER_MAX		4 // (Intervalll�nge = 20 ms / IRQ-Zeit = 4 ms)
#define TIMER_TWI_READY_TIMEOUT_MS	2


//External Data
extern volatile bool timer_slot_0;
extern volatile bool timer_slot_1;
extern volatile bool timer_slot_2;
extern volatile bool timer_slot_3;
extern volatile uint8_t timer_twi_ready_timeout;

//External Functions
extern void timer_init(void);



#endif /* TIMER_H_ */
