/*
 * base64.h
 *
 *  Created on: Feb 21, 2015
 *      Author: alexandermertens
 */

#ifndef BASE64_H_
#define BASE64_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include "stdbool.h"
#include "stdint.h"

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define BASE64_BUFFER_SIZE 33    // (8*4 encoded bytes + 1 '\0'-byte)

#define BASE64_ENC_ERROR    0

/* * external objects            * */
extern uint8_t base64_encode_buffer[BASE64_BUFFER_SIZE];

/* * external functions          * */
extern uint8_t base64_encode(const uint8_t * data, const uint8_t data_size);
//extern void base64_decode(void); //TODO: Implement decode function

#endif /* BASE64_H_ */
