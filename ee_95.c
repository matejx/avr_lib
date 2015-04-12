/**
@file		ee_95.c
@brief		ST95P08 EEPROM routines
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
@note		Written for ST95P08. Compatibility with other SPI EE devices unknown.
*/

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"
#include "ee95.h"
#include "hwdefs.h"

#define EE95_DELAY asm("nop\nnop\nnop\nnop\nnop\n")

/** @privatesection */

void ee95_cs(uint8_t a) {
	EE95_DELAY;

	if( a ) {
		EE95_CS_PORT &= ~_BV(EE95_CS_BIT);
	} else {
		EE95_CS_PORT |=  _BV(EE95_CS_BIT);
	}

	EE95_DELAY;
}

void ee95_wp(uint8_t a) {
	EE95_DELAY;

	if( a ) {
		EE95_WP_PORT &= ~_BV(EE95_WP_BIT);
	} else {
		EE95_WP_PORT |=  _BV(EE95_WP_BIT);
	}

	EE95_DELAY;
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

/** @publicsection */

/**
@brief Initializes SPI, CS and EE write protect pin
@param[in]	fdiv	SPI fdiv
*/
void ee95_init(uint8_t fdiv)
{
	// make CS pin an output and set it high
	DDR(EE95_CS_PORT) |= _BV(EE95_CS_BIT);
	ee95_cs(0);

	// make WP pin an output and set it low (protect)
	DDR(EE95_WP_PORT) |= _BV(EE95_WP_BIT);
	ee95_wp(1);

	spi_init(fdiv);
}

/**
@brief Read from EE.
@param[in]	adr		Starting byte address
@param[out]	buf		Caller provided buffer for data
@param[in]	len		Number of bytes to read (len <= sizeof(buf))
*/
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

/**
@brief Write to EE.
@param[in]	adr		Byte address
@param[in]	d		Data byte
@warning Can hang waiting for correct SR.
*/
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
