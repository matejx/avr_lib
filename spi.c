/**

SPI methods are not interrupt driven - they wait until SPI operation completes.
If you're using CMT and would prefer switching to another task while SPI operation
is in progress, you can define SPI_USE_CMT in swdefs.h. This requires CMT_MUTEX_FUNC.

Also, SS (CS) is not controlled by these methods. It's the responsibility of the user.

@file		spi.c
@brief		SPI routines
@author		Matej Kogovsek
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "swdefs.h"
#include "hwdefs.h"

#ifdef SPI_USE_CMT
	#warning SPI using cmt
	#include "cmt.h"
	struct cmt_mutex spi_mutex;
#endif

#ifndef SPCR0
	#define SPCR0 SPCR
	#define SPE0 SPE
	#define MSTR0 MSTR
	#define SPSR0 SPSR
	#define SPIF0 SPIF
	#define CPOL0 CPOL
	#define CPHA0 CPHA
	#define SPDR0 SPDR
#endif

#ifdef SPI_PORT
	#define SCK_DDR DDR(SPI_PORT)
	#define MOSI_DDR DDR(SPI_PORT)
	#define MISO_DDR DDR(SPI_PORT)
#endif

/**
@brief Initialize SPI interface.
@param[in]	fdiv		Baudrate prescaler
*/
void spi_init(uint8_t fdiv)
{
	if( SPCR0 & _BV(SPE0) ) return;

	#ifdef SPI_USE_CMT
	spi_mutex.ac = 0;
	#endif

	// make SCK, MOSI pins outputs and MISO an input
	SCK_DDR |= _BV(SCK_BIT);
	MOSI_DDR |= _BV(MOSI_BIT);
	MISO_DDR &= ~_BV(MISO_BIT);

	// init SPI, MSB first, SCK low when idle
	SPCR0 = _BV(SPE0) | _BV(MSTR0) | (fdiv & 3);
	SPSR0 = (fdiv >> 2) & 1;
}

/**
@brief Send and receive byte (NSS not controlled)
@param[in]	d			Byte to send
@return byte received
*/
uint8_t spi_rw(uint8_t d)
{
	#ifdef SPI_USE_CMT
	cmt_acquire(&spi_mutex);
	#endif

	SPCR0 |= _BV(MSTR0);
	SPDR0 = d;
	while( !(SPSR0 & _BV(SPIF0)) ) {
		#ifdef SPI_USE_CMT
		cmt_delay_ticks(0);
		#endif
	}
	d = SPDR0;

	#ifdef SPI_USE_CMT
	cmt_release(&spi_mutex);
	#endif

	return d;
}

/**
@brief Set SPI mode
@param[in] m	Mode (0..3)
*/
void spi_mode(uint8_t m)
{
	if( m > 3 ) return;

	uint8_t d = SPCR0 & ~(_BV(CPOL0) | _BV(CPHA0));
	if( m >= 2 ) d |= _BV(CPOL0);
	if( m & 1 ) d |= _BV(CPHA0);
	SPCR0 = d;
}

/**
@brief Set SPI speed
@param[in] fdiv	Clock divider
*/
void spi_fdiv(uint8_t fdiv)
{
	if( fdiv > 7 ) return;

	SPCR0 = (SPCR0 & ~3) | (fdiv & 3);
	SPSR0 = (fdiv >> 2) & 1;
}

// ------------------------------------------------------------------
// INTERRUPTS
// ------------------------------------------------------------------

ISR(SPI_STC_vect)
{
//
}
