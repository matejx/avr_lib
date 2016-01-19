/**

Define SCL_PORT, SCL_BIT, SDA_PORT and SDA_BIT in hwdefs.h.

You can also override SWI2C_DELAY in swdefs.h.
Defining SWI2C_REVBITS in swdefs.h will transfer LSB first instead of MSB.
Defining SWI2C_RAWADR in swdefs.h will transfer i2c address exactly as given, without setting R/W bit (bit 0).

@file		swi2c.c
@brief		Software (bitbang) I2C.
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hwdefs.h"
#include "swdefs.h"

#ifndef SWI2C_DELAY
#define SWI2C_DELAY _delay_us(swi2c_br)
#endif

/** @privatesection */

uint8_t swi2c_br;
//uint8_t _BVM[] = {1,2,4,8,16,32,64,128};

void SCL_HI(void) {
	DDR(SCL_PORT) &= ~_BV(SCL_BIT);
}

void SCL_LO(void) {
	SCL_PORT &= ~_BV(SCL_BIT);
	DDR(SCL_PORT) |= _BV(SCL_BIT);
}

void SDA_HI(void) {
	DDR(SDA_PORT) &= ~_BV(SDA_BIT);
}

void SDA_LO(void) {
	SDA_PORT &= ~_BV(SDA_BIT);
	DDR(SDA_PORT) |= _BV(SDA_BIT);
}

void swi2c_start(void)
{
	SCL_HI();
	SDA_HI();
	SWI2C_DELAY;
	SDA_LO();
	SWI2C_DELAY;
	SCL_LO();
}  // SCL and SDA are low after swi2c_start

// ------------------------------------------------------------------

void swi2c_stop(void)
{
	SDA_LO();
	SWI2C_DELAY;
	SCL_HI();
	SWI2C_DELAY;
	SDA_HI();
}  // SCL and SDA are high after swi2c_stop

// ------------------------------------------------------------------

uint8_t swi2c_putc(const uint8_t d)
{
	uint8_t i;
	#ifdef SWI2C_REVBITS
	for( i = 0; i < 8; i++ ) {
	#else
	for( i = 7; i < 8; i-- ) {
	#endif
		SWI2C_DELAY;
		if( d & _BV(i) ) {
			SDA_HI();
		} else {
			SDA_LO();
		}
		SWI2C_DELAY;
		SCL_HI();
		SWI2C_DELAY;
		SWI2C_DELAY;
		SCL_LO();
	}
	// get ACK
	SWI2C_DELAY;
	SDA_HI();
	SWI2C_DELAY;
	SCL_HI();
	SWI2C_DELAY;
	uint8_t r = PIN(SDA_PORT) & _BV(SDA_BIT);	// read ack
	SWI2C_DELAY;
	SCL_LO();
	return r;
}  // SCL low, SDA high after swi2c_putc

// ------------------------------------------------------------------

uint8_t swi2c_getc(void)
{
	uint8_t d = 0;
	uint8_t i;

	#ifdef SWI2C_REVBITS
	for( i = 0; i < 8; i++ ) {
	#else
	for( i = 7; i < 8; i-- ) {
	#endif
		SWI2C_DELAY;
		SWI2C_DELAY;
		SCL_HI();
		SWI2C_DELAY;
		if( PIN(SDA_PORT) & _BV(SDA_BIT) ) { d |= _BV(i); }	// read SDA
		SWI2C_DELAY;
		SCL_LO();
	}
	// gen ACK
	SWI2C_DELAY;
	SDA_LO();
	SWI2C_DELAY;
	SCL_HI();
	SWI2C_DELAY;
	SWI2C_DELAY;
	SCL_LO();

	return d;
}  // both SCL and SDA are low after swi2c_getc

/** @publicsection */

void i2c_init(uint8_t br)
{
	swi2c_br = br;
}

uint8_t i2c_readbuf(const uint8_t adr, uint8_t* const data, const uint8_t len)
{
	swi2c_start();
	#ifdef SWI2C_RAWADR
	swi2c_putc(adr);
	#else
	swi2c_putc(adr | 1);
	#endif
	for( uint8_t i = 0; i < len; ++i ) data[i] = swi2c_getc();
	swi2c_stop();

	return 0;
}

uint8_t i2c_writebuf(const uint8_t adr, uint8_t* const data, const uint8_t len)
{
	swi2c_start();
	#ifdef SWI2C_RAWADR
	swi2c_putc(adr);
	#else
	swi2c_putc(adr & 0xfe);
	#endif
	for( uint8_t i = 0; i < len; ++i ) swi2c_putc(data[i]);
	swi2c_stop();

	return 0;
}
