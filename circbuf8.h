// ------------------------------------------------------------------
// --- circbuf.c - interrupt safe circular buffer routines        ---
// ---                                                            ---
// ---                                 7.nov.2010, Matej Kogovsek ---
// ------------------------------------------------------------------

#ifndef MAT_CIRCBUF8_H
#define MAT_CIRCBUF8_H

#include <inttypes.h>

struct cbuf8_t
{
	uint8_t* buf;
	uint8_t head;
	uint8_t tail;
	uint8_t len;
	uint8_t size;
};

void cbuf8_clear(volatile struct cbuf8_t* cb, uint8_t* const p, const uint8_t s);
uint8_t cbuf8_put(volatile struct cbuf8_t* cb, const uint8_t d);
uint8_t cbuf8_get(volatile struct cbuf8_t* cb, uint8_t* const d);

#endif