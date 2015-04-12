/**

SPI methods are not interrupt driven - they wait until SPI operation completes.
If you're using CMT and would prefer switching to another task while SPI operation
is in progress, you can define SPI_USE_CMT in swdefs.h. This requires CMT_MUTEX_FUNC.

Also, SS (CS) is not controlled by these methods. It's the responsibility of the user.

@file		spi.c
@brief		SPI routines
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-stm32f1-lib
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
	#define SPDR0 SPDR
	#define SPIF0 SPIF
#endif

/**
@brief Initialize SPI interface.

Although SPI can have different clock phase and polarity, I have never ran across anything that uses other
than low polarity and 1st edge phase. Therefore these parameters are implied and not variable. As are 8 bit
words and MSB first.
@param[in]	fdiv		Baudrate prescaler, F_CPU dependent
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

// ------------------------------------------------------------------
// INTERRUPTS
// ------------------------------------------------------------------

ISR(SPI_STC_vect)
{
//
}
