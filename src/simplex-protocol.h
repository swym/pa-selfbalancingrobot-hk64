/*
 * simplex-linklayer.h
 *
 *  Created on: Feb 20, 2014
 *      Author: alexandermertens
 */

#ifndef SIMPLEX_LINKLAYER_H_
#define SIMPLEX_LINKLAYER_H_

/* *** INCLUDES ************************************************************** */

/* * system headers              * */

#include <stdint.h>
#include <stdbool.h>

/* * local headers               * */

/* *** DECLARATIONS ********************************************************** */

/* * external type and constants * */
#define SIMPLEX_PROTOCOL_MAX_FRAME_SIZE 10

#define SIMPLEX_PROTOCOL_MODE_RX 1
#define SIMPLEX_PROTOCOL_MODE_TX 2

//Frame Typen, die kein ACK ben�tigen
#define SIMPLEX_PROTOCOL_FRAME_TYPE_DATA	0x01

//Frame Typen, die ein ACK ben�tigen
#define SIMPLEX_PROTOCOL_FRAME_TYPE_CONTROL	0x81
#define SIMPLEX_PROTOCOL_FRAME_TYPE_ACK		0x88

#define SIMPLEX_PROTOCOL_CONTROL_STOP		0
#define SIMPLEX_PROTOCOL_CONTROL_START		1


#define SIMPLEX_PROTOCOL_BUFFER_EMPTY		0
#define SIMPLEX_PROTOCOL_BUFFER_ARRIVAL		1
#define SIMPLEX_PROTOCOL_BUFFER_CRCERROR	2

typedef uint8_t simplex_rx_status_t;

typedef uint8_t simplex_protocol_frame_type_t;
typedef uint8_t simplex_protocol_seq_nr_t;
typedef uint8_t simplex_protocol_ack_nr_t;
typedef uint8_t simplex_protocol_data_length_t;
typedef uint8_t simplex_protocol_data_ptr_t;
typedef uint8_t simplex_protocol_crc_t;

typedef struct {
	simplex_protocol_frame_type_t frame_type;
	simplex_protocol_seq_nr_t seq_nr;
	simplex_protocol_ack_nr_t ack_nr;
	simplex_protocol_data_length_t data_length;
	simplex_protocol_data_ptr_t data[SIMPLEX_PROTOCOL_MAX_FRAME_SIZE];
	simplex_protocol_crc_t crc;

} simplex_protocol_frame_t;

/* * external objects            * */

extern uint8_t simplex_protocol_mode;

/* * external functions          * */

/*
 * init()
 * send()
 * simplex_linklayer_tick()
 * receive()s
 */


extern void simplex_protocol_init();
extern void simplex_protocol_tick();

extern bool simplex_protocol_send(uint8_t, uint8_t, uint8_t *);

extern simplex_rx_status_t simplex_protocol_rx_status();
extern void simplex_protocol_rx_clear();
extern simplex_protocol_frame_t * simplex_protocol_get_rx_frame();


#endif /* SIMPLEX_LINKLAYER_H_ */
