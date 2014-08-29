// ------------------------------------------------------------------
// --- i2cee.c - I2C EE routines for 24Cxxx                       ---
// ---                                                            ---
// ---                                30.nov.2013, Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>

#include "i2c.h"
#include "string.h"

#define EE24_I2C_ADR 0xa0

// ------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ------------------------------------------------------------------

void ee24_init(uint8_t br)
{
	i2c_init(br);
}

// ------------------------------------------------------------------

uint8_t ee24_wr(uint16_t adr, uint8_t* buf, uint8_t len)
{
	if( len > 64 ) return 1;

	uint8_t buf2[len+2];
	buf2[0] = adr >> 8;
	buf2[1] = adr;

	if( len ) memcpy(buf2+2, buf, len);

	return i2c_writebuf(EE24_I2C_ADR, buf2, len+2);
}

// ------------------------------------------------------------------

uint8_t ee24_rd(uint16_t adr, uint8_t* buf, uint8_t len)
{
	if( ee24_wr(adr, 0, 0) ) return 1;
	return i2c_readbuf(EE24_I2C_ADR, buf, len);
}

// ------------------------------------------------------------------
