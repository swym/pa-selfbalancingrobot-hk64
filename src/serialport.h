/*
 * serialport.h
 *
 *  Created on: Jun 20, 2015
 *      Author: alexandermertens
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */
#include <stdbool.h>
#include <stdint.h>


/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define STRING_BUFFER_SIZE		40

/* * external objects            * */

/* * external functions          * */
extern void serialport_init(uint32_t baudrate);
extern void serialport_set_baudrate(uint32_t baudrate);
extern void serialport_enable_port(bool enable);

extern bool serialport_get_string(char * target, uint8_t target_size);

#endif /* SERIALPORT_H_ */
