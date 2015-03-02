// ------------------------------------------------------------------
// --- ee95.c - serial EE routines for ST95P08                    ---
// ---                                                            ---
// ---                                30.nov.2013, Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "ee95.h"

#define DDR(x) (*(&x - 1))
#define PIN(x) (*(&x - 2))

#define EE95_DELAY asm("nop\nnop\nnop\nnop\nnop\n")

// ------------------------------------------------------------------
// PRIVATE FUNCTIONS
// ------------------------------------------------------------------

void ee95_cs(uint8_t a) {
	SPIEE_DELAY;

	if( a ) {
		SPIEE_CS_PORT &= ~_BV(SPIEE_CS_BIT);
	} else {
		SPIEE_CS_PORT |=  _BV(SPIEE_CS_BIT);
	}

	SPIEE_DELAY;
}

void ee95_wp(uint8_t a) {
	SPIEE_DELAY;

	if( a ) {
		SPIEE_WP_PORT &= ~_BV(SPIEE_WP_BIT);
	} else {
		SPIEE_WP_PORT |=  _BV(SPIEE_WP_BIT);
	}

	SPIEE_DELAY;
}

void ee95_wren(void)
{
	ee95_cs(1);
	spi_rw(0x06);	// WREN
	ee95_cs(0);
}

uint8_t ee95_rdsr(void)
{
	ee95_cs(1);
	spi_rw(0x05);	// RDSR
	uint8_t sr = spi_rw(0);
	ee95_cs(0);
	return sr;
}

// ------------------------------------------------------------------
// PUBLIC FUNCTIONS
// ------------------------------------------------------------------

void ee95_init(void)
{
	// make CS pin an output and set it high
	DDR(SPIEE_CS_PORT) |= _BV(SPIEE_CS_BIT);
	ee95_cs(0);

	// make WP pin an output and set it low (protect)
	DDR(SPIEE_WP_PORT) |= _BV(SPIEE_WP_BIT);
	ee95_wp(1);

	spi_init();
}

// ------------------------------------------------------------------

void ee95_rd(uint16_t adr, uint8_t* buf, uint16_t len)
{
	ee95_cs(1);

	spi_rw( ((adr >> 5) & 0x18) | 0x03 ); // READ instruction containing A9 and A8
	spi_rw( adr & 0xff ); // address A7..A0

	while( len ) {
		*buf = spi_rw(0);
		++buf;
		--len;
	}

	ee95_cs(0);
}

// ------------------------------------------------------------------

void ee95_wr(uint16_t adr, uint8_t d)
{
	ee95_wp(0);
	ee95_wren();

	ee95_cs(1);
	spi_rw( ((adr >> 5) & 0x18) | 0x02 ); // WRITE instruction containing A9 and A8
	spi_rw( adr & 0xff ); // address A7..A0
	spi_rw(d);
	ee95_cs(0);

	ee95_wp(1);

	while( ee95_rdsr() & 1 ) {
		_delay_ms(1);
	}
}

// ------------------------------------------------------------------
