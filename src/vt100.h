/*
 * vt100.h
 *
 *  Created on: Dec 5, 2013
 *      Author: alexandermertens
 */


#ifndef VT100_H_
#define VT100_H_

/* *** INCLUDES ************************************************************** */

/* system headers              */ 
#include <stdint.h>

/* local headers               */



/* *** DECLARATIONS ********************************************************** */

/* external type and constants */
#define VT100_INPUT_BUFFER_SIZE 80


/* external objects            */

/* external functions          */
extern void		vt100_clear_screen(void);
extern void		vt100_clear_input_buffer(void);
extern void		vt100_clear_all(void);
extern void		vt100_set_cursor_top_left(void);
extern void		vt100_set_cursor_on_position(uint8_t, uint8_t); /* uint8_t x, uint8_t y */
extern char		vt100_get_choice(void);
extern void		vt100_get_string(char *str, uint8_t max_len);
extern uint16_t	vt100_get_integer(const uint16_t, const uint16_t, const uint16_t); /* uint16_t current_value, uint16_t min_range, uint16_t max_range */



#endif /* VT100_H_ */



