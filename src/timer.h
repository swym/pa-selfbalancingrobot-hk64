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

#define TIMER_SLOT_COUNTER_MAX 4 // (Intervalll�nge = 20 ms / IRQ-Zeit = 4 ms)-1 -> 5-1



//External Data
extern volatile bool timer_compare_reached;
extern volatile uint8_t timer_slot_cnt;

//External Functions
extern void timer_init(void);



#endif /* TIMER_H_ */
