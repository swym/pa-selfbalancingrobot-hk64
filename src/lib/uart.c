#include <avr/io.h>
#include <stdio.h>
#include "uart.h"

/*****************************************************************************
    Hier koennen Anpassungen an die Hardware vorgenommen werden.
*****************************************************************************/
#if defined (__AVR_ATmega8__) /*Getestet unter NiboBee V1.0*/
#   define MYUBRRH  UBRRH
#   define MYUBRRL  UBRRL
#   define MYUCSRA  UCSRA
#   define MYUCSRB  UCSRB
#   define MYUCSRC  UCSRC
#   define MYUDR    UDR
#   define MYRXC    RXC
#   define MYTXC    TXC
#   define MYUDRE   UDRE
#   define MYFE     FE
#   define MYDOR    DOR
#   define MYUSE_U2X USE_U2X
#   define MYRXEN   RXEN
#   define MYTXEN   TXEN
#elif defined (__AVR_ATmega16__) /*Getestet unter NiboBee V1.0*/
#   define MYUBRRH  UBRRH
#   define MYUBRRL  UBRRL
#   define MYUCSRA  UCSRA
#   define MYUCSRB  UCSRB
#   define MYUCSRC  UCSRC
#   define MYUDR    UDR
#   define MYRXC    RXC
#   define MYTXC    TXC
#   define MYUDRE   UDRE
#   define MYFE     FE
#   define MYDOR    DOR
#   define MYUSE_U2X USE_U2X
#   define MYRXEN   RXEN
#   define MYTXEN   TXEN
#elif defined (__AVR_ATmega32__) /*Getestet unter Fuchs-Board, Version 3*/
#   define MYUBRRH  UBRRH
#   define MYUBRRL  UBRRL
#   define MYUCSRA  UCSRA
#   define MYUCSRB  UCSRB
#   define MYUCSRC  UCSRC
#   define MYUDR    UDR
#   define MYRXC    RXC
#   define MYTXC    TXC
#   define MYUDRE   UDRE
#   define MYFE     FE
#   define MYDOR    DOR
#   define MYUSE_U2X USE_U2X
#   define MYRXEN   RXEN
#   define MYTXEN   TXEN
#elif defined (__AVR_ATmega64__) /*Getestet unter ATM64P-Board, Version 1*/
#   define MYUBRRH  UBRR0H
#   define MYUBRRL  UBRR0L
#   define MYUCSRA  UCSR0A
#   define MYUCSRB  UCSR0B
#   define MYUCSRC  UCSR0C
#   define MYUDR    UDR0
#   define MYRXC    RXC0
#   define MYTXC    TXC0
#   define MYUDRE   UDRE0
#   define MYFE     FE0
#   define MYDOR    DOR0
#   define MYUSE_U2X USE_U2X0
#   define MYRXEN   RXEN0
#   define MYTXEN   TXEN0
#elif defined (__AVR_ATmega2560__) /*Getestet unter Dorobo V0.3*/
#   define MYUBRRH  UBRR3H
#   define MYUBRRL  UBRR3L
#   define MYUCSRA  UCSR3A
#   define MYUCSRB  UCSR3B
#   define MYUCSRC  UCSR3C
#   define MYUDR    UDR3
#   define MYRXC    RXC3
#   define MYTXC    TXC3
#   define MYUDRE   UDRE3
#   define MYFE     FE3
#   define MYDOR    DOR3
#   define MYUSE_U2X USE_U2X3
#   define MYRXEN   RXEN3
#   define MYTXEN   TXEN3
#else
#   error "Please define the wished layout for your machine."
    ARCHITECTURE_undefined = Hier_ist_ein_Fehler;
#endif

/*Im AVR-Studio sollte F_CPU definiert werden unter
  Project->Configuration Options->Frequency*/
#ifndef F_CPU
#  error "You can't use uart.c without defining F_CPU"
   /*Error fuehrt nicht immer zum Abbruch beim Uebersetzen. Das hier schon:*/
   F_CPU_nicht_definiert = Hier_ist_ein_Fehler;
#endif /*F_CPU*/

#define USE_U2X    1
#define RX_BUFSIZE 80

static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);

/* **************************************************************************
    M O D U L I N T E R N E   V A R I A B L E N
*****************************************************************************/
static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
static FILE stderr_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


/* **************************************************************************
    G L O B A L E   F U N K T I O N E N
*****************************************************************************/
uint8_t UART_char_received(void)
{
    return (MYUCSRA & (1<<MYRXC)) ? !0 : 0;
}


void UART_clr_rx_buf(void)
{
    uint8_t tmp;

    while (MYUCSRA & (1<<MYRXC)) {
        tmp = MYUDR;
    }
}


void UART_init(uint32_t baudrate)
{
#if F_CPU < 2000000UL && defined(MYUSE_U2X)
    MYUCSRA = _BV(U2X); /* improve baud rate error by using 2x clk */
    MYUBRRL = (F_CPU / (8UL * baudrate)) - 1;
#else
    MYUBRRL = (F_CPU / (16UL * baudrate)) - 1;
#endif
    MYUCSRB = _BV(MYTXEN) | _BV(MYRXEN); /* tx/rx enable */

    stdout = stdin = &uart_str;
    stderr = &stderr_str;
}


/* **************************************************************************
    M O D U L I N T E R N E   F U N K T I O N E N
*****************************************************************************/
static int uart_putchar(char c, FILE *stream)
{
    /*XXX: Hier kann jemand einen Summer, Licht oder sonstwas aktivieren.*/
    if (c == '\a') {
        return 0;
    }

    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(MYUCSRA, MYUDRE);
    MYUDR = c;

    return 0;
}


/*
 * Receive a character from the UART Rx.
 *
 * This features a simple line-editor that allows to delete and
 * re-edit the characters entered, until either CR or NL is entered.
 * Printable characters entered will be echoed using uart_putchar().
 *
 * Editing characters:
 *
 * . \b (BS) or \177 (DEL) delete the previous character
 * . ^u kills the entire input buffer
 * . ^w deletes the previous word
 * . ^r sends a CR, and then reprints the buffer
 * . \t will be replaced by a single space
 *
 * All other control characters will be ignored.
 *
 * The internal line buffer is RX_BUFSIZE (80) characters long, which
 * includes the terminating \n (but no terminating \0).  If the buffer
 * is full (i. e., at RX_BUFSIZE-1 characters in order to keep space for
 * the trailing \n), any further input attempts will send a \a to
 * uart_putchar() (BEL character), although line editing is still
 * allowed.
 *
 * Input errors while talking to the UART will cause an immediate
 * return of -1 (error indication).  Notably, this will be caused by a
 * framing error (e. g. serial line "break" condition), by an input
 * overrun, and by a parity error (if parity was enabled and automatic
 * parity recognition is supported by hardware).
 *
 * Successive calls to uart_getchar() will be satisfied from the
 * internal buffer until that buffer is emptied again.
 */
static int uart_getchar(FILE *stream)
{
    uint8_t c;
    char *cp, *cp2;
    static char b[RX_BUFSIZE];
    static char *rxp;

    if (rxp == 0) {
        for (cp = b;;) {
            loop_until_bit_is_set(MYUCSRA, MYRXC);
            if (MYUCSRA & _BV(MYFE))
                return _FDEV_EOF;
            if (MYUCSRA & _BV(MYDOR))
                return _FDEV_ERR;
            c = MYUDR;
            /* behaviour similar to Unix stty ICRNL */
            if (c == '\r')
                c = '\n';
            if (c == '\n') {
                *cp = c;
                uart_putchar(c, stream);
                rxp = b;
                break;
            }
            else if (c == '\t') {
                c = ' ';
            }

            if ((c >= (uint8_t)' ' && c <= (uint8_t)'\x7e') ||
                c >= (uint8_t)'\xa0')
            {
                if (cp == b + RX_BUFSIZE - 1)
                    uart_putchar('\a', stream);
                else {
                    *cp++ = c;
                    uart_putchar(c, stream);
                }
                continue;
            }

            switch (c) {
            case 'c' & 0x1f:
                return -1;

            case '\b':
            case '\x7f':
                if (cp > b) {
                    uart_putchar('\b', stream);
                    uart_putchar(' ', stream);
                    uart_putchar('\b', stream);
                    cp--;
                }
                break;

            case 'r' & 0x1f:
                uart_putchar('\r', stream);
                for (cp2 = b; cp2 < cp; cp2++)
                    uart_putchar(*cp2, stream);
                break;

            case 'u' & 0x1f:
                while (cp > b) {
                    uart_putchar('\b', stream);
                    uart_putchar(' ', stream);
                    uart_putchar('\b', stream);
                    cp--;
                }
                break;

            case 'w' & 0x1f:
                while (cp > b && cp[-1] != ' ') {
                    uart_putchar('\b', stream);
                    uart_putchar(' ', stream);
                    uart_putchar('\b', stream);
                    cp--;
                }
                break;
            } /*switch (c)*/
        } /*for (cp = b;;)*/
    } /*if (rxp == 0)*/

    c = *rxp++;
    if (c == '\n')
        rxp = 0;

    return c;
}
