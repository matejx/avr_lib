// ------------------------------------------------------------------
// --- circbuf.c - interrupt safe circular buffer routines        ---
// ---                                                            ---
// ---                                 7.nov.2010, Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "circbuf8.h"

// ------------------------------------------------------------------
// initializes (clears) circ buf

void cbuf8_clear(volatile struct cbuf8_t* cb, uint8_t* const p, const uint8_t s)
{
	uint8_t g = SREG;
	cli();

	cb->buf = p;
	cb->size = s;
	cb->head = 0;
	cb->tail = 0;
	cb->len = 0;
	
	SREG = g;
}

// ------------------------------------------------------------------
// inserts an element

uint8_t cbuf8_put(volatile struct cbuf8_t* cb, const uint8_t d)
{
	uint8_t g = SREG;
	cli();
	
	if (cb->len == cb->size) { 
		SREG = g;
		return 0;
	}

	cb->buf[cb->tail] = d;
	cb->tail++;
	if(cb->tail == cb->size) { cb->tail = 0; }
	cb->len++;
	
	SREG = g;
	return 1;
}

// ------------------------------------------------------------------
// gets the next element

uint8_t cbuf8_get(volatile struct cbuf8_t* cb, uint8_t* const d)
{
	uint8_t g = SREG;
	cli();
	
	if (cb->len == 0) { 
		SREG = g;
		return 0; 
	}

	if( d ) {	// if d is null, cbuf_get can be used to check for data in buffer
		*d = cb->buf[cb->head];
		cb->head++;
		if(cb->head == cb->size) { cb->head = 0; }
		cb->len--;
	}

	SREG = g;
	return 1;
}

// ------------------------------------------------------------------
