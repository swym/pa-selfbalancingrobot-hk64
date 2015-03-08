/*
 * base64.c
 *
 *  Created on: Feb 21, 2015
 *      Author: alexandermertens
 */

//QUELLE: http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

/* *** INCLUDES ************************************************************* */
#include "base64.h"

/* * system headers              * */
#include <stdio.h>

/* * local headers               * */


/* *** DEFINES ************************************************************** */


/* *** DECLARATIONS ********************************************************* */

/* * local type and constants    * */
#define BASE64_PADBYTE		'='
#define ENDOFSTRING			'\0'
#define SIXBIT_MASK			0x3F

/* * local used ext. module objs * */
uint8_t base64_encode_buffer[BASE64_BUFFER_SIZE];

/* * local objects               * */
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								'w', 'x', 'y', 'z', '0', '1', '2', '3',
								'4', '5', '6', '7', '8', '9', '+', '/'};

/* * local function declarations * */


/* *** FUNCTION DEFINITIONS ************************************************* */


uint8_t base64_encode(const uint8_t * data, const uint8_t data_size)
{
	uint8_t i;					//generic loop counter
	uint8_t data_index;			//index of input data array
	uint8_t enc_index;			//index of encoded data array
	uint32_t threebyte_buffer;	//temporary butter that old three input bytes to encode
	uint8_t enc_table_index;	//index for looking up encoded byte
	uint8_t padbytes;			//counter of pad bytes
	uint8_t encoded_size;		//stores calcuated encoded data size


	//calculate size of encoded data + one '\0'-byte
	encoded_size = (4 * ((data_size + 2) / 3)) + 1;

	//if buffer wouldn't fit; return with error
	if(!(encoded_size < BASE64_BUFFER_SIZE)) {
		return BASE64_ENC_ERROR;
	}

	//init indices and padbyte counter
	enc_index = 0;
	data_index = 0;
	padbytes = 0;

	//repeat for all input data bytes;
	//inc of 'data_index' occurs within the loop
	while((data_index < data_size)) {

		//push first inputbyte into bytebuffer
		threebyte_buffer = (uint32_t)(data[data_index++]) << 16;

		//push second inputbyte into bytebuffer and inc padbyte counter, if all
		//inputbytes processed
		if(data_index < data_size) {
			threebyte_buffer |= (uint32_t)(data[data_index++]) << 8;
		} else {
			padbytes++;
		}

		//push second inputbyte into bytebuffer and inc padbyte counter, if all
		//inputbytes processed
		if(data_index < data_size) {
			threebyte_buffer |= (uint32_t)(data[data_index++]);
		} else {
			padbytes++;
		}

		//convert the three 8-bit symbols to four 6-bit symbols and lookup
		//encoding byte from encoding table
		enc_table_index = (threebyte_buffer >> 18) & SIXBIT_MASK;
		base64_encode_buffer[enc_index++] = encoding_table[enc_table_index];

		enc_table_index = (threebyte_buffer >> 12) & SIXBIT_MASK;
		base64_encode_buffer[enc_index++] = encoding_table[enc_table_index];

		enc_table_index = (threebyte_buffer >> 6) & SIXBIT_MASK;
		base64_encode_buffer[enc_index++] = encoding_table[enc_table_index];

		enc_table_index = (threebyte_buffer) & SIXBIT_MASK;
		base64_encode_buffer[enc_index++] = encoding_table[enc_table_index];
	}

	//replace up to two padbytes at the end of encoded data
	for(i = 0; i < padbytes;i++) {
		base64_encode_buffer[enc_index - 1 - i] = BASE64_PADBYTE;
	}

	//put end of string symbol at the end of buffered data so
	//printf can easily output data
	base64_encode_buffer[enc_index++] = ENDOFSTRING;

	return enc_index;
}

