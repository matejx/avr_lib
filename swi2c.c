/**
@file		swi2c.c
@brief		Software (bitbang) I2C.
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>

#include "hwdefs.h"

#define i2cDelay asm("nop\nnop\n")

/** @privatesection */

uint8_t i2c_error;

void i2cStart(void)
{
	SCL_PORT |= _BV(SCL_BIT);			// make SCL high
	DDR(SCL_PORT) |= _BV(SCL_BIT);		// make SCL an output

	SDA_PORT |= _BV(SDA_BIT);			// make SDA high
	DDR(SDA_PORT) |= _BV(SDA_BIT);		// make SDA an output

	i2cDelay;
	SDA_PORT &= ~_BV(SDA_BIT);			// SDA low
	i2cDelay;
	SCL_PORT &= ~_BV(SCL_BIT);			// SCL low
}  // both SCL and SDA are outputs and low after i2cStart

// ------------------------------------------------------------------

void i2cStop(void)
{
	DDR(SDA_PORT) |= _BV(SDA_BIT);		// make SDA an ouput
	SDA_PORT &= ~_BV(SDA_BIT);			// put SDA low
	i2cDelay;
	DDR(SCL_PORT) &= ~_BV(SCL_BIT);		// make SCL an input (goes high by pullup)
	i2cDelay;
	DDR(SDA_PORT) &= ~_BV(SDA_BIT);		// make SDA an input (goes high by pullup)
}  // both SCL and SDA are inputs and high after i2cStop

// ------------------------------------------------------------------

uint8_t i2cPutByte(const uint8_t d)
{
	DDR(SDA_PORT) |= _BV(SDA_BIT);		// make SDA an output

	uint8_t i;
	for( i = 7; i < 8; i-- ) {
		i2cDelay;
		if( d & _BV(i) ) {
			SDA_PORT |= _BV(SDA_BIT);
		} else {
			SDA_PORT &= ~_BV(SDA_BIT);
		}
		i2cDelay;
		SCL_PORT |= _BV(SCL_BIT);
		i2cDelay;
		i2cDelay;
		SCL_PORT &= ~_BV(SCL_BIT);
	}
	// ACK
	i2cDelay;
	DDR(SDA_PORT) &= ~_BV(SDA_BIT);		// make SDA an input (goes high by pullup)
	i2cDelay;
	SCL_PORT |= _BV(SCL_BIT);			// SCL high
	i2cDelay;
	uint8_t r = PIN(SDA_PORT) & _BV(SDA_BIT);	// read ack
	i2cDelay;
	SCL_PORT &= ~_BV(SCL_BIT);			// SCL low
	return r;
}  // SCL output and low, SDA input (high by pullup)

// ------------------------------------------------------------------

uint8_t i2cGetByte(void)
{
	uint8_t d = 0;
	uint8_t i;

	DDR(SDA_PORT) &= ~_BV(SDA_BIT);		// make SDA an input

	for( i = 7; i < 8; i-- ) {
		i2cDelay;
		i2cDelay;
		SCL_PORT |= _BV(SCL_BIT);		// SCL high
		i2cDelay;
		if( PIN(SDA_PORT) & _BV(SDA_BIT) ) { d |= _BV(i); }	// read SDA
		i2cDelay;
		SCL_PORT &= ~_BV(SCL_BIT);		// SCL low
	}

	i2cDelay;
	SDA_PORT &= ~_BV(SDA_BIT);			// SDA low
	DDR(SDA_PORT) |= _BV(SDA_BIT);		// make SDA an output
	i2cDelay;
	SCL_PORT |= _BV(SCL_BIT);			// SCL high
	i2cDelay;
	i2cDelay;
	SCL_PORT &= ~_BV(SCL_BIT);			// SCL low

	return d;
}  // SCL output and low, SDA output and low

/** @publicsection */

void i2c_init(uint8_t br)
{
	// dummy
}

void i2c_writebyte(const uint8_t adr, const uint8_t data)
{
	i2cStart();
	i2cPutByte(adr & 0xfe);
	i2cPutByte(data);
	i2cStop();
}

// ------------------------------------------------------------------

void i2c_writebyte2(const uint8_t adr, const uint8_t data1, const uint8_t data2)
{
	i2cStart();
	i2cPutByte(adr & 0xfe);
	i2cPutByte(data1);
	i2cPutByte(data2);
	i2cStop();
}

// ------------------------------------------------------------------

uint8_t i2c_writereadbyte(const uint8_t adr, uint8_t data)
{
	i2cStart();
	i2cPutByte(adr & 0xfe);
	i2cPutByte(data);
	i2cStart();
	i2cPutByte(adr | 1);
	data = i2cGetByte();
	i2cStop();
	return data;
}

// ------------------------------------------------------------------

uint8_t i2c_readbyte(const uint8_t adr)
{
	i2cStart();
	i2cPutByte(adr | 1);
	uint8_t data = i2cGetByte();
	i2cStop();
	return data;
}
