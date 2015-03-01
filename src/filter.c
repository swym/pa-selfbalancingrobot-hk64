/*
 * filters.c
 *
 *  Created on: Jul 16, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "filter.h"

/* * system headers              * */

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */

/* * local function declarations * */

/* *** FUNCTION DEFINITIONS ************************************************* */


void filter_moving_generic_average_put_element(
		filter_moving_generic_average_t * average,
		int16_t new_value)
{
	uint8_t i;
	int32_t elements_sum = 0;

	// iterate over the (weights - 1) to aging existing elements
	for(i = average->weights_count - 1; i > 0; i--) {
		// move elements to left (aging them; drop last element)
		average->elements[i] = average->elements[i - 1];
		// elements_sum them up with their weight
		elements_sum += ((int32_t)average->elements[i]) * average->weights[i];
	}
	//add newest element, apply weight to element and add to sum
	average->elements[0] = new_value;
	elements_sum += ((int32_t)average->elements[0]) * average->weights[0];

	//divide elements_sum with weights_sum and save calculated average in structure
	average->avg = (int16_t)(elements_sum / average->weights_sum);
}


void filter_moving_generic_average_init(
		filter_moving_generic_average_t * average,
		uint8_t * weights,
		int16_t init_value)
{
	uint8_t i;

	average->weights_sum = 0;
	average->weights_count = 0;

	for(i = 0; i < FILTER_MOVING_GENERIC_WEIGHTS_COUNT;i++) {
		if(weights[i] > 0) {
			average->weights[i] = weights[i];				//copy weight
			average->weights_sum += average->weights[i];	//upate sum
			average->weights_count++;						//update counter
		} else {
			average->weights[i] = 0;
		}
	}

	for(i = 0; i < average->weights_count; i++) {
		filter_moving_generic_average_put_element(average, init_value);
	}
}

void filter_moving_generic_average_flush(
		filter_moving_generic_average_t * average)
{
	uint8_t i;

	for(i = 0; i < average->weights_count; i++) {
		filter_moving_generic_average_put_element(average, 0);
	}
}

void filter_moving_average_put_element(
		filter_moving_average_t *average,
		int16_t new_value)
{
	//remove oldest value from sum and add newest one
	average->elements_sum -= average->elements[average->index];
	average->elements_sum += new_value;

	//add new value in elements array
	average->elements[average->index] = new_value;

	//calculate new average
	average->avg = average->elements_sum / FILTER_MOVING_AVERAGE_ELEMENT_COUNT;

	//move index to next element
	if(average->index < FILTER_MOVING_AVERAGE_ELEMENT_COUNT - 1) {
		average->index++;
	} else {
		average->index = 0;
	}
}

void filter_moving_average_init(
		filter_moving_average_t *average,
		int16_t init_value)
{
	uint8_t i;

	for(i = 0;i < FILTER_MOVING_AVERAGE_ELEMENT_COUNT;i++) {
		filter_moving_average_put_element(average, init_value);
	}
}

void filter_moving_average_flush(
		filter_moving_average_t *average)
{
	uint8_t i;

	for(i = 0;i < FILTER_MOVING_AVERAGE_ELEMENT_COUNT;i++) {
		filter_moving_average_put_element(average, 0);
	}
}


//TODO: implement a generic filter based on shift operations
/*
void filters_weighted_average_put_element(weighted_average_t *average, int16_t value)
{
	int32_t elements_sum = 0;						//sum of all elements and their weights
	int8_t i;										//index for-loop


	//iterate over elements and move elements to next
	for(i = WEIGHTED_AVERAGE_ELEMENT_COUNT - 1;i > 0;i--) {
		average->elements[i] = average->elements[i - 1];
		elements_sum += ((int32_t)(average->elements[i]) << weighted_average_weights[i]);
	}

	//add newst element
	average->elements[0] = value;

	//apply weight to element and add to sum
	elements_sum += ((int32_t)(average->elements[0]) << weighted_average_weights[0]);

	//devide sum
	average->mean = elements_sum >> weighted_average_devisor;
}

*/

