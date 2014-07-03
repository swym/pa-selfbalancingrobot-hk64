/*
 * simplex_linklayer.c
 *
 *  Created on: Feb 21, 2014
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************* */

/* * system headers              * */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>			//For debug outputs
#include <util/crc16.h>
#include <avr/io.h>

/* * local headers               * */
#include "lib/rfm12.h"
#include "lib/hamming.h"
#include "simplex-protocol.h"

/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
#define FRAME_BUFFER_STATUS_NOT_SEND   0
#define FRAME_BUFFER_STATUS_SEND       1

//Frame Buffer = 2 * (Data + Header (5)) (2 mal wegen FEC)
#define SIMPLEX_PROTOCOL_FRAME_BUFFER_SIZE (2*(SIMPLEX_PROTOCOL_MAX_FRAME_SIZE+5))

typedef uint8_t buffer_status;

/* * local objects               * */
uint8_t simplex_protocol_mode;


//argregation of the frame an its byte-array representation
typedef struct {
	buffer_status status;
	simplex_protocol_frame_t frame;
	uint8_t byte_array[SIMPLEX_PROTOCOL_FRAME_BUFFER_SIZE];
	uint8_t bytecount;
} frame_buffer_t;

frame_buffer_t send_buffer;
frame_buffer_t receive_buffer;


/* * local function declarations * */
void simplex_protocol_send_frame();
uint8_t simplex_protocol_crc(simplex_protocol_frame_t *);
void simplex_protocol_marshal_frame(frame_buffer_t *);
bool simplex_protocol_unmarshal_frame(frame_buffer_t *);


/* *** FUNCTION DEFINITIONS ************************************************* */

// true if data could queued in send buffer
bool
simplex_protocol_send(
	simplex_protocol_frame_type_t frame_type,
	simplex_protocol_data_length_t data_length,
	uint8_t * data)
{
	if(data_length > SIMPLEX_PROTOCOL_MAX_FRAME_SIZE) {
		return false;
	}
	//init frame header
	send_buffer.frame.frame_type = frame_type;
	send_buffer.frame.seq_nr = 0;
	send_buffer.frame.ack_nr = 0;

	//set data_length
	send_buffer.frame.data_length = data_length;
	//Copy data to frame
	memcpy(&send_buffer.frame.data, data, data_length);

	//calculate crc
	send_buffer.frame.crc = simplex_protocol_crc(&send_buffer.frame);

	//pack send_buffer
	simplex_protocol_marshal_frame(&send_buffer);

	//mark frame as ready to send
	send_buffer.status = FRAME_BUFFER_STATUS_NOT_SEND;
	return true;
}

uint8_t simplex_protocol_crc(simplex_protocol_frame_t *f)
{

	uint8_t crc = 0;
	uint8_t i;

	//crc header fields
	crc = _crc_ibutton_update(crc, f->frame_type);
	crc = _crc_ibutton_update(crc, f->seq_nr);
	crc = _crc_ibutton_update(crc, f->ack_nr);
	crc = _crc_ibutton_update(crc, f->data_length);

	//crc data
	for(i = 0;i < f->data_length;i++) {
		crc = _crc_ibutton_update(crc, f->data[i]);
	}

	return crc;
}

//Marshal frame and add FEC using hamming table
void simplex_protocol_marshal_frame(frame_buffer_t *fb)
{
	uint8_t i;

	fb->bytecount = 0;

	//Add Header
	fb->byte_array[fb->bytecount++] = Hamming_encHigh(fb->frame.frame_type);
	fb->byte_array[fb->bytecount++] = Hamming_encLow(fb->frame.frame_type);

	fb->byte_array[fb->bytecount++] = Hamming_encHigh(fb->frame.seq_nr);
	fb->byte_array[fb->bytecount++] = Hamming_encLow(fb->frame.seq_nr);

	fb->byte_array[fb->bytecount++] = Hamming_encHigh(fb->frame.ack_nr);
	fb->byte_array[fb->bytecount++] = Hamming_encLow(fb->frame.ack_nr);

	fb->byte_array[fb->bytecount++] = Hamming_encHigh(fb->frame.data_length);
	fb->byte_array[fb->bytecount++] = Hamming_encLow(fb->frame.data_length);


	//Add Data
	for(i = 0;i < fb->frame.data_length;i++) {
		fb->byte_array[fb->bytecount++] = Hamming_encHigh(fb->frame.data[i]);
		fb->byte_array[fb->bytecount++] = Hamming_encLow(fb->frame.data[i]);
	}

	//Add crc
	fb->byte_array[fb->bytecount++] = Hamming_encHigh(fb->frame.crc);
	fb->byte_array[fb->bytecount++] = Hamming_encLow(fb->frame.crc);
}

//true if reveived frame could unmarshalled.
bool simplex_protocol_unmarshal_frame(frame_buffer_t *fb)
{
	uint8_t i;
	uint8_t tmp;
	uint8_t array_index = 0;

	tmp = Hamming_decHigh(fb->byte_array[array_index++]);
	tmp |= Hamming_decLow(fb->byte_array[array_index++]);
	fb->frame.frame_type  = tmp;

	tmp = Hamming_decHigh(fb->byte_array[array_index++]);
	tmp |= Hamming_decLow(fb->byte_array[array_index++]);
	fb->frame.seq_nr  = tmp;

	tmp = Hamming_decHigh(fb->byte_array[array_index++]);
	tmp |= Hamming_decLow(fb->byte_array[array_index++]);
	fb->frame.ack_nr  = tmp;

	tmp = Hamming_decHigh(fb->byte_array[array_index++]);
	tmp |= Hamming_decLow(fb->byte_array[array_index++]);
	fb->frame.data_length  = tmp;


	//if data_length field is greater than the limit, something went wrong while
	//transmission
	if(fb->frame.data_length > SIMPLEX_PROTOCOL_MAX_FRAME_SIZE) {
		return false;
	}

	//Add Data
	for(i = 0;i < fb->frame.data_length;i++) {

		tmp = Hamming_decHigh(fb->byte_array[array_index++]);
		tmp |= Hamming_decLow(fb->byte_array[array_index++]);
		fb->frame.data[i] = tmp;
	}

	tmp = Hamming_decHigh(fb->byte_array[array_index++]);
	tmp |= Hamming_decLow(fb->byte_array[array_index++]);
	fb->frame.crc = tmp;

	return true;
}


void simplex_protocol_send_frame()
{
	if(send_buffer.status == FRAME_BUFFER_STATUS_NOT_SEND) {

		//	printf("send frame: \n");

		rfm12_tx(send_buffer.bytecount,
		42,
		send_buffer.byte_array);

		send_buffer.status = FRAME_BUFFER_STATUS_SEND;
	}
}


simplex_rx_status_t simplex_protocol_rx_status()
{
	return receive_buffer.status;
}

void simplex_protocol_init()
{
	send_buffer.status = SIMPLEX_PROTOCOL_BUFFER_EMPTY;
	receive_buffer.status = SIMPLEX_PROTOCOL_BUFFER_EMPTY;
}

void simplex_protocol_rx_clear()
{
	receive_buffer.status = SIMPLEX_PROTOCOL_BUFFER_EMPTY;
}

simplex_protocol_frame_t * simplex_protocol_get_rx_frame()
{
	return &receive_buffer.frame;
}

void simplex_linklayer_receive_frame()
{
	uint8_t *rx_buffer_ptr;
	uint8_t i;
	uint8_t buf_len;

	if(receive_buffer.status == SIMPLEX_PROTOCOL_BUFFER_EMPTY) {

		// Copy data from mac layer to link layer
		rx_buffer_ptr = rfm12_rx_buffer();
		buf_len = rfm12_rx_len();

		for(i = 0;i < buf_len;i++) {
			receive_buffer.byte_array[i] = rx_buffer_ptr[i];
		}
		receive_buffer.bytecount = i;

		//convert from byte array to struct
		simplex_protocol_unmarshal_frame(&receive_buffer);

		//check crc and set status
		if(simplex_protocol_crc(&receive_buffer.frame) == receive_buffer.frame.crc) {
			receive_buffer.status = SIMPLEX_PROTOCOL_BUFFER_ARRIVAL;
		} else {
			receive_buffer.status = SIMPLEX_PROTOCOL_BUFFER_CRCERROR;
		}
	}

	//clear mac rx buffer
	rfm12_rx_clear();
}

void simplex_protocol_tick()
{
	//rfm12 needs to be called from your main loop periodically.
	//it checks if the rf channel is free (no one else transmitting), and then
	//sends packets, that have been queued by rfm12_tx above.
	rfm12_tick();

	simplex_protocol_send_frame();

	//MAC Layer received a frame
	if(rfm12_rx_status() == STATUS_COMPLETE) {

		simplex_linklayer_receive_frame();
	}

}
