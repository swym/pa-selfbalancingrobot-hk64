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

#define TIMER_SLOT_COUNTER_MAX 4 // (Intervalll�nge = 20 ms / IRQ-Zeit = 4 ms)



//External Data
extern volatile bool timer_slot_1;
extern volatile bool timer_slot_2;
extern volatile bool timer_slot_3;

//External Functions
extern void timer_init(void);



#endif /* TIMER_H_ */
