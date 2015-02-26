/*
 * filters.c
 *
 *  Created on: Jul 16, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */
#include "filters.h"

/* * system headers              * */
#include <stdio.h>

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */

/* * local objects               * */
static uint8_t weighted_average_weights[] = {3, 2, 1, 0, 0};	//element << weights[i] -> element * (8, 4, 2, 1, 1)
static uint8_t weighted_average_devisor = 4;				//sum of left shifted weights
/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */

//8, 2, 1, 0, 0, 0, 0, 0;
void filters_moving_generic_average_init(
		filters_moving_generic_average_t * average,
		uint8_t * weights,
		uint16_t init_value)
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
		filters_moving_generic_average_put_element(average, init_value);
	}
}

void filters_moving_generic_average_flush(filters_moving_generic_average_t * average)
{
	uint8_t i;

	for(i = 0; i < average->weights_count; i++) {
		filters_moving_generic_average_put_element(average, 0);
	}
}

void filters_moving_generic_average_put_element(
		filters_moving_generic_average_t * average,
		int16_t new_value)
{

	//int32_t elem = 0;
	//int32_t mul = 0;

	uint8_t i;
	int32_t element_sum = 0;


//	printf("summing:\n");
	//iterate over elements and move elements to next
	for(i = average->weights_count - 1; i > 0; i--) {
		average->elements[i] = average->elements[i - 1];							//move elements to left (older them)
//		elem = average->elements[i];
//		mul = ((int32_t)average->elements[i]) * average->weights[i];
		//element_sum +=  ((int32_t)(average->elements[i]) * average->weights[i]);	//sum them up with their weight
		element_sum += ((int32_t)average->elements[i]) * average->weights[i];
//		printf("[e:%6ld m:%6ld s:%6ld]     ",elem, mul, element_sum);
	}
	//add newest element
	//apply weight to element and add to sum
	average->elements[0] = new_value;
	//element_sum += (int32_t)((average->elements[0]) * average->weights[0]);
	//printf("%6ld\n", element_sum);
//	elem = average->elements[0];
//	mul = ((int32_t)average->elements[0]) * average->weights[0];
	element_sum += ((int32_t)average->elements[0]) * average->weights[0];
//	printf("[e:%6ld m:%6ld s:%6ld]\n",elem, mul, element_sum);
/*
	printf("elem: ");
	for(i = 0; i < average->weights_count; i++) {
		printf("%6d ", average->elements[i]);
	}
	printf("\n");

	printf("w:    ");
	for(i = 0; i < average->weights_count; i++) {
		printf("%6d ", average->weights[i]);
	}
	printf("\n");
*/
	average->avg = (int16_t)(element_sum / average->weights_sum);			//devide sum with weights
//	printf("avg: %6d\n", average->avg);
//	printf("\n\n");
}

void filters_moving_average_put_element(moving_average_t *average, int16_t value)
{
	average->elements_sum -= average->elements[average->index];	//zu UEberschreibenen Wert aus der Summe loeschen
	average->elements_sum += value;							//Neuen Wert der Summe hinzufuegen

	average->elements[average->index] = value;		//Neuen Wert im Array eintragen.

	//Neuen Mittelwert berrechnen
	average->mean = average->elements_sum / MOVING_AVERAGE_ELEMENT_COUNT;

	//Index auf die neue Stelle schieben
	if(average->index < MOVING_AVERAGE_ELEMENT_COUNT - 1) {
		average->index++;
	} else {
		average->index = 0;
	}
}

//TODO: Replace with a more performant implementation
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

void filters_smooth_put_element(smooth_t * average, int16_t value)
{
	average->smoothed = ((value - average->smoothed) / average->factor) + average->smoothed;
}

/*
	for(i = 0;i < WEIGHTED_AVERAGE_ELEMENT_COUNT - 1;i++) {
		elements_sum += average->elements[i] * weighted_average_weights[i+1];
		average->elements[i+1] = average->elements[i];
	}

	average->elements[0] = value;
	elements_sum += (average->elements[0] * weighted_average_weights[0]);
	average->mean = (elements_sum / weighted_average_devisor);
*/

/*//FIXME: More complex but more efficient?
void filters_weighted_average_put_element(weighted_average_t *average, int16_t value)
{
	int64_t elements_sum;							//sum of all elements and their weights
	uint8_t i;										//index for-loop
	uint8_t weight_index = 1;						//start with second weight in for-loop for summing
	uint8_t element_index = average->head_index;	//init element_index with last head element

	//iterate (element_count - 1) times...
	for(i = 0;i < WEIGHTED_AVERAGE_ELEMENT_COUNT - 1;i++) {

		//sum elements with corresponding weight
		elements_sum += average->elements[element_index] << weighted_average_weights[weight_index];

		//select next weight
		weight_index++;

		//if at bottom of element_array set element_index to top
		if(element_index > 0) {
			element_index--;
		} else {
			element_index = WEIGHTED_AVERAGE_ELEMENT_COUNT - 1;
		}
	}

	//add newest value to element sum
	elements_sum += value << weighted_average_weights[0];

	//make mean
	average->mean = elements_sum >> weighted_average_devisor;

	//correct head_index for new value
	if(average->head_index < WEIGHTED_AVERAGE_ELEMENT_COUNT - 1) {
		average->head_index++;
	} else {
		average->head_index = 0;
	}

	//add new value to set of elements
	average->elements[average->head_index] = value;
}
*/
