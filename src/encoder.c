/************************************************************************/
/*                                                                      */
/*            Drehgeber mit wackeligem Rastpunkt dekodieren             */
/*  Siehe http://www.mikrocontroller.net/articles/Drehgeber             */
/*                                                                      */
/************************************************************************/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <assert.h>
#include <stdlib.h>
#include "encoder.h"

/* Mapping der Encoder-Anschluesse auf die uC-Ports; ggf. anpassen! */
#define ENCODER_CNT 2

#define ENCODER_PORT	PORTF
#define ENCODER_DDR		DDRF
#define ENCODER_PIN		PINF

#define	ENCODER_M1_PIN1	PINF4
#define	ENCODER_M1_PIN2	PINF5
#define	ENCODER_M2_PIN1	PINF6
#define	ENCODER_M2_PIN2	PINF7

static const uint8_t phase_[ENCODER_CNT][2] = {
		{ENCODER_M1_PIN1, ENCODER_M1_PIN2},
		{ENCODER_M2_PIN1, ENCODER_M2_PIN2}};

/* Drehgeberbewegung zwischen zwei Auslesungen im Hauptprogramm. */
static volatile int8_t enc_delta_[ENCODER_CNT];

#if 0
  /* Dekodertabelle fuer wackeligen Rastpunkt: halbe Aufloesung */
  const int8_t table_[16] PROGMEM = {  0,  0, -1,  0,
		  	  	  	  	  	  	  	   0,  0,  0,  1,
									   1,  0,  0,  0,
									   0, -1,  0,  0};
#else
  /* Dekodertabelle fuer normale Drehgeber: volle Aufloesung */
  const int8_t table_[16] PROGMEM = {  0,  1, -1,  0,
		  	  	  	  	  	  	  	  -1,  0,  0,  1,
									   1,  0,  0, -1,
									   0, -1,  1,  0};
#endif


/*This function can also be used to reinit the system.*/
void encoder_init(void)
{
    /*Set encoder pins as input.*/
	ENCODER_DDR &= ~(_BV(ENCODER_M1_PIN1) | _BV(ENCODER_M1_PIN2) |
					 _BV(ENCODER_M2_PIN1) | _BV(ENCODER_M2_PIN2));

    /*Enable pullups*/
	ENCODER_PORT |= (_BV(ENCODER_M1_PIN1) | _BV(ENCODER_M1_PIN2) |
					 _BV(ENCODER_M2_PIN1) | _BV(ENCODER_M2_PIN2));

	/*init encoder deltas*/
    for (uint_fast8_t i = 0; i < ENCODER_CNT; i++)
        enc_delta_[i] = 0;
}


int8_t encoder_read_delta(enum encoder_id encoder)
{
    assert(encoder < ENCODER_CNT);
    int8_t val;

    val = enc_delta_[encoder];
    enc_delta_[encoder] = 0;

    return val;
}


void encoder_handler(void) /* call this each millisec, e.g. by interrupt */
{
    static uint_fast8_t last[ENCODER_CNT]; /* remember last state */

    for (uint_fast8_t i = 0; i < ENCODER_CNT; i++) {

        last[i] = (last[i] << 2) & 0x0F;

        if (bit_is_set(ENCODER_PIN, phase_[i][0])) {
        	last[i] |= 2;
        }

        if (bit_is_set(ENCODER_PIN, phase_[i][1])) {
        	last[i] |= 1;
        }
        enc_delta_[i] -= pgm_read_byte(&table_[last[i]]);
    }
}
