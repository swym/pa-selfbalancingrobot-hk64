#ifndef ENCODER_H
#define ENCODER_H
#include <stdint.h>


enum encoder_id {
  ENCODER_M1,
  ENCODER_M2
};

void encoder_init(void);
int8_t encoder_read_delta(enum encoder_id encoder);
void encoder_handler(void);

#endif /*ENCODER_H*/
/**
   \file encoder.h
   \brief Auslesen der Encoder-Scheiben.

   Mit dem Modul <em>Encoder</em> k�nnen die gez�hlten Pulse von zwei
   Encoder-Scheiben gelesen werden. Das im Projekt verwendete
   Pin-Mapping entnimmt man der folgenden Grafik:

   <IMG src="../pics/encoder.png" alt="Encoder-Mapping">

   Dieses Modul liest die Encoder-Werte automatisch �ber eine im
   Hintergrund laufende Funktion an. Hierzu mu� jede Millisekunde
   die Funktion _ENC_handler() von einem Interrupt aufgerufen
   werden. Siehe hierzu auch den in main.c implementierten
   Interrupt <em>ISR(TIMER0_COMP_vect).</em>

   Vor dem ersten Benutzen mu� die Funktion ENC_init() aufgerufen werden.

   <h2>Verwendete Resourcen</h2>

   F�r Ein- und Ausgabe werden die folgenden Ports verwendet:

   <table>
     <tr>
       <th>Anschlu�</th>
       <th>Richtung</th>
       <th>Name</th>
       <th>Bedeutung</th>
     </tr>
     <tr>
       <td>PIND2</td>
       <td>I</td>
       <td>E1A</td>
       <td>Encoder 1 A</td>
     </tr>
     <tr>
       <td>PIND6</td>
       <td>I</td>
       <td>E1B</td>
       <td>Encoder 1 B</td>
     </tr>
     <tr>
       <td>PIND3</td>
       <td>I</td>
       <td>E2A</td>
       <td>Encoder 2 A</td>
     </tr>
     <tr>
       <td>PIND7</td>
       <td>I</td>
       <td>E2B</td>
       <td>Encoder 2 B</td>
     </tr>
   </table>


   \enum ENC_ID
   \brief Identifizierer f�r einen einzelnen Encoder.

   Hiermit kann definiert werden, ob man auf den rechten oder linken Encoder
   zugreifen m�chte.


   \fn void ENC_init(void)
   \brief Initialisierung des Encoder-Moduls.

   Diese Funktion mu� vor dem Benutzen des Moduls aufgerufen werden.


   \fn int8_t ENC_read(enum ENC_ID encoder)
   \brief Gez�hlte Schritte eines Encoders auslesen.
   \param encoder Auswahl eines Encoders.
   \return Die gez�hlten Schritte des gew�hlten Encoders.

   Diese Funktion liefert die Anzahl der seit dem letzten Aufruf gez�hlten
   Encoder-Schritte zur�ck. Sofern der Aufruf regelm��ig erfolgt, kann auf
   diese Weise die Geschwindigkeit ermittelt werden.

   Das Z�hlen der einzelnen Schritte erfolgt im Hintergrund durch
   _ENC_handler().


   \fn void _ENC_handler(void)
   \brief �berwacht den Zustand der Encoder.

   Diese Funktion realisiert im Hintergrund die Encoder-�berwachung und mu�
   z.B. jede Millisekunde durch einen Interrupt aufgerufen werden.
 */

// Local Variables:
// coding: iso-latin-1
// mode: C
// End:
