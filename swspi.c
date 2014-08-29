// ------------------------------------------------------------------
// --- swspi.c - bitbang SPI                                      ---
// ---                                                            ---
// ---                                16.jan.2014, Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <util/delay.h>

#include "hwdefs.h"

void swspi_init(void)
{
	DDR(MOSI_PORT) |= _BV(MOSI_BIT);
	
	DDR(SCK_PORT) |=  _BV(SCK_BIT);
	SCK_PORT &= ~_BV(SCK_BIT);
}

uint8_t swspi_rw(uint8_t b)
{
	uint8_t r = 0;

	uint8_t i;
	for( i = 0; i < 8; ++i ) {
		r <<= 1;
		if( b & _BV(7) ) {
			MOSI_PORT |= _BV(MOSI_BIT);
		} else {
			MOSI_PORT &= ~_BV(MOSI_BIT);
		}
		_delay_us(SPI_DELAY_US);
		SCK_PORT |= _BV(SCK_BIT);
		_delay_us(SPI_DELAY_US);
		SCK_PORT &= ~_BV(SCK_BIT);
		_delay_us(SPI_DELAY_US);
		if( PIN(MISO_PORT) & _BV(MISO_BIT) ) r |= 1;
		b <<= 1;
	}

	return r;
}
