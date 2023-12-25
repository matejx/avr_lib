/**

All USART data transmission is interrupt driven. Received data is put into a FIFO (provided by circbuf8.c).
Data to be transmitted is likewise put into a FIFO. Memory for both FIFOs is provided by the caller on init.

@file		serque.c
@brief		Buffered USART routines
@author		Matej Kogovsek
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

#include "circbuf8.h"
#include "swdefs.h"

#ifndef UBRR0H
	#define UBRR0H UBRRH
	#define UBRR0L UBRRL
	#define UCSR0A UCSRA
	#define UCSR0B UCSRB
	#define UCSR0C UCSRC
	#define RXCIE0 RXCIE
	#define RXEN0 RXEN
	#define TXEN0 TXEN
	#define UCSZ01 UCSZ1
	#define UCSZ00 UCSZ0
	#define UDR0 UDR
	#define UDRIE0 UDRIE
#endif

#ifndef USART0_UDRE_vect
	#define USART0_UDRE_vect USART_UDRE_vect
#endif

#ifndef USART0_RX_vect
	#ifdef USART_RX_vect
		#define USART0_RX_vect USART_RX_vect
	#else
		#define USART0_RX_vect USART_RXC_vect
	#endif
#endif

#ifdef URSEL
	#define _URSEL _BV(URSEL)
#else
	#define _URSEL 0
#endif

// VOLATILE QUEUES !!! VERY IMPORTANT !!!
static volatile struct cbuf8_t uart_rxq[2];
static volatile struct cbuf8_t uart_txq[2];

/**
@brief Init USART.
@param[in]	n			USART peripheral number (1..2)
@param[in]	br			Baudrate (i.e. BAUD_115200 for 115.2k)
@param[in]	txb			Pointer to caller allocated TX buffer
@param[in]	txs			sizeof(txb)
@param[in]	rxb			Pointer to caller allocated RX buffer
@param[in]	rxs			sizeof(rxs)
*/
void ser_init(const uint8_t n, const uint16_t br, uint8_t* txb, uint8_t txs, uint8_t* rxb, uint8_t rxs)
{
	cbuf8_clear(&uart_txq[n], txb, txs);
	cbuf8_clear(&uart_rxq[n], rxb, rxs);

	if( n ) {
#ifdef UDRE1
		UBRR1H = br >> 8;
		UBRR1L = br;
		UCSR1A = 0;		// normal UART speed, no multi-proc comm mode
		UCSR1B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);	// enable RX/TX, enable RX int
		UCSR1C = _BV(UCSZ01) | _BV(UCSZ00);	// 8 bit characters
#endif
	} else {
		UBRR0H = br >> 8;
		UBRR0L = br;
		UCSR0A = 0;		// normal UART speed, no multi-proc comm mode
		UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);	// enable RX/TX, enable RX int
		UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | (_URSEL);	// 8 bit characters
	}
}

/**
@brief Deinit USART.
@param[in]	n			USART peripheral number (1..2)
*/
void ser_shutdown(const uint8_t n)
{
	if( n ) {
#ifdef UDRE1
		UCSR1B = 0;
#endif
	} else {
		UCSR0B = 0;
	}
}

/**
@brief Flush rx buffer.
@param[in]	n			USART peripheral number (1..2)
*/
void ser_flush_rxbuf(const uint8_t n)
{
	cbuf8_clear(&uart_rxq[n], uart_rxq[n].buf, uart_rxq[n].size);
}

/**
@brief Enqueue a byte to the serial queue for transmission.
@param[in]	n			USART peripheral number (1..2)
@param[in]	a			Byte to transmit
*/
uint8_t ser_putc(const uint8_t n, const char a)
{
	while( !cbuf8_put(&uart_txq[n], a) ) { // put data to queue
		if( bit_is_clear(SREG, SREG_I) ) { return 0; }  // deadloop guard
	}

	if( n ) {  // enable data register empty interrupt
#ifdef UDRE1
		UCSR1B |= _BV(UDRIE0);
#endif
	} else {
		UCSR0B |= _BV(UDRIE0);
	}

	return 1;
}

/**
@brief Get a byte from the serial queue.
@param[in]	n			USART peripheral number (1..2)
@param[out]	d			Pointer to uint8_t where received data is put
@return Same as cbuf8_get
*/
uint8_t ser_getc(const uint8_t n, uint8_t* const d)
{
	return cbuf8_get(&uart_rxq[n], d);
}

#ifdef SER_NEED_PUTSP
/**
@brief Send a string from pgmem.
@param[in]	n			USART peripheral number (1..2)
@param[in]	s			Zero terminated string to send
*/
void ser_puts_P(const uint8_t n, const PGM_P s)
{
	uint8_t c;
	while ((c = pgm_read_byte(s++)))	{
		ser_putc(n, c);
	}
}
#endif

#ifdef SER_NEED_PUTS
/**
@brief Send a string from memory.
@param[in]	n			USART peripheral number (1..2)
@param[in]	s			Zero terminated string to send
*/
void ser_puts(const uint8_t n, const char* s)
{
	while (*s)	{
		ser_putc(n, *s);
		s++;
	}
}
#endif

#ifdef SER_NEED_PUTSESC
void ser_puts_esc(const uint8_t n, const char* s)
{
	while( *s ) {
		if( *s == '\n') {
			ser_puts(n, "\\n");
		} else
		if( *s == '\r') {
			ser_puts(n, "\\r");
		} else
		if( *s == '\t') {
			ser_puts(n, "\\t");
		} else {
			ser_putc(n, *s);
		}
		s++;
	}
}
#endif

#ifdef SER_NEED_PUTI
/**
@brief Send int in the specified radix r of minlen w prepended by char c.
@param[in]	n			USART peripheral number (1..2)
@param[in]	a			int
@param[in]	r			Radix
@param[in]	l			Min width
@param[in]	c			Prepending char to achieve min width
*/
void ser_puti_lc(const uint8_t n, const uint32_t a, const uint8_t r, uint8_t l, char c)
{
	char s[10];

	ltoa(a, s, r);

	while( l > strlen(s) ) {
		ser_putc(n, c);
		l--;
	}

	ser_puts(n, s);
}
#endif

#ifdef SER_NEED_PUTF
/**
@brief Send float with specified precision.

This is a naive implementation of ftoa with the primary goal of keeping the code size to a minimum.
Thus the function has certain limitations, i.e. since the integral part of the argument is cast to
integer, it must be in int range (+-2 * 10^9). If you need certainty, take a look at dtostrf.

@param[in]	n			USART peripheral number (1..2)
@param[in]	f			float
@param[in]	prec		Number of decimals
*/
void ser_putf(const uint8_t n, float f, uint8_t prec)
{
	if( f < 0 ) {
		f = -f;
		ser_putc(n, '-');
	}
	ser_puti_lc(n, f, 10, 0, 0);
	ser_putc(n, '.');
	f = f - (int)f;
	uint8_t i = prec;
	while( i-- ) f *= 10;
	ser_puti_lc(n, f, 10, prec, '0');
}
#endif

#ifdef SER_NEED_PUTF2
#include <math.h>
// takes an extra 100 bytes of code space due to modff
void ser_putf(const uint8_t n, float f, uint8_t prec)
{
	if( f < 0 ) {
		f = -f;
		ser_putc(n, '-');
	}
	float fi, ff;
	ff = modff(f, &fi);
	ser_puti_lc(n, fi, 10, 0, 0);
	ser_putc(n, '.');
	uint8_t i = prec;
	while( i-- ) ff *= 10;
	ser_puti_lc(n, ff, 10, prec, '0');
}
#endif

#ifdef SER_NEED_TXDONE
/**
@brief Returns true if tx queue is empty, false otherwise.
@param[in]	n			USART peripheral number (1..2)
*/
uint8_t ser_txdone(const uint8_t n)
{
	return (uart_txq[n].len == 0);
}
#endif


/** @privatesection */

// ------------------------------------------------------------------
// INTERRUPTS
// ------------------------------------------------------------------

ISR(USART0_RX_vect)
{
	uint8_t d = UDR0;            	// read receive FIFO whether rxq empty or not!

	cbuf8_put(&uart_rxq[0], d);
}

#ifdef UDRE1
ISR(USART1_RX_vect)
{
	uint8_t d = UDR1;            	// read receive FIFO whether rxq empty or not!
	cbuf8_put(&uart_rxq[1], d);
}
#endif

// ----------------------------

ISR(USART0_UDRE_vect)
{
	uint8_t d;
	if( cbuf8_get(&uart_txq[0], &d) ) {
		UDR0 = d;
	} else {
		UCSR0B &= ~_BV(UDRIE0);     // no more data to send, disable UDR empty int
	}
}

#ifdef UDRE1
ISR(USART1_UDRE_vect)
{
	uint8_t d;
	if( cbuf8_get(&uart_txq[1], &d) ) {
		UDR1 = d;
	} else {
		UCSR1B &= ~_BV(UDRIE0);     // no more data to send, disable UDR empty int
	}
}
#endif
