#ifndef UART_H
#define UART_H
#include <inttypes.h>

extern uint8_t uart_char_received(void);
extern void uart_clr_rx_buf(void);
extern void uart_init(uint32_t baudrate);

extern uint8_t uart_getstring(char* Buffer, uint8_t MaxLen);


#endif /*UART_H*/
/**
   \file uart.h
   \brief Initialisierung einer seriellen Konsole.

   Durch das Modul <em>uart</em> werden die Standard-Datenstr�me
   <a href="http://de.wikipedia.org/wiki/Stdin">stdin</a>,
   <a href="http://de.wikipedia.org/wiki/Stdout">stdout</a> und
   <a href="http://de.wikipedia.org/wiki/Stderr">stderr</a> mit dem
   <em>USART-</em>Ger�t des <a href="http://www.atmel.com/">ATmega32</a>
   verbunden. Physikalisch steht diese serielle Schnittstelle nach au�en gem��
   dem <a href="http://de.wikipedia.org/wiki/EIA-232">EIA-232-</a>Standard
   zur Verf�gung und kann somit mit einem PC verbunden werden. Standardm��ig
   wird das System in der Betriebsart <em>8N1 ohne Handshake</em> eingerichtet.

   Nach dem Aufruf von UART_init() stehen zu Ein- und Ausgabezwecken die in der
   <a href="http://de.wikipedia.org/wiki/Header-Datei">stdio.h-</a>Datei
   definierten Standard-Ein- und Ausgabefunktionen der
   <a href="http://www.nongnu.org/avr-libc/">AVR Libc</a> zur Verf�gung, also
   z.B. printf, fgets usw. Siehe auch <a href=
   "http://de.wikibooks.org/wiki/C-Programmierung:_Einfache_Ein-_und_Ausgabe">
   Einf�hrung</a>.

   Zum Verbinden mit einem PC wird zudem ein Terminalprogramm, wie z.B. <a
   href="http://hp.vector.co.jp/authors/VA002416/teraterm.html">Tera Term</a>,
   <a href="http://de.wikipedia.org/wiki/HyperTerminal">HyperTerminal</a>
   oder <a href="http://de.wikipedia.org/wiki/Minicom">Minicom</a>, ben�tigt.


   \fn void UART_init(uint32_t baudrate)
   \brief Initialisierung einer seriellen Konsole.
   \param baudrate Gew�nschte Baudrate der seriellen Konsole.

   Um die serielle Schnittstelle einzurichten, muss diese Funktion aufgerufen
   werden. F�r den Parameter <em>baudrate</em> sollten gebr�uchliche Werte
   wie 9600, 19200 und, je nach Kabell�nge, auch 115200 verwendet werden.
   Hierbei ist das USART-Kapitel, insbesondere die am Kapitelende aufgef�hrten
   Daten der Tabelle
   &quot;Examples of UBRR Settings for Commonly Used Oscillator
   Frequencies&quot; der Herstellerdokumentation des Mikrocontrollers zu
   beachten, nach dem sich nicht f�r jede Oszillatorfrequenz g�ltige
   Toleranzwerte f�r alle �bertragungsraten ergeben werden.
 */

// Local Variables:
// coding: iso-latin-1
// mode: C
// End:
