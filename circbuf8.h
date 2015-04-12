#ifndef MAT_CIRCBUF8_H
#define MAT_CIRCBUF8_H

#include <inttypes.h>

struct cbuf8_t
{
	uint8_t* buf; /**< pointer to buffer */
	uint8_t head; /**< index of head */
	uint8_t tail; /**< index of tail */
	uint8_t len; /**< data length */
	uint8_t size; /**< buffer size */
};

void cbuf8_clear(volatile struct cbuf8_t* cb, uint8_t* const p, const uint8_t s);
uint8_t cbuf8_put(volatile struct cbuf8_t* cb, const uint8_t d);
uint8_t cbuf8_get(volatile struct cbuf8_t* cb, uint8_t* const d);

#endif
