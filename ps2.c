/**

PS2 device routines

@file		ps2.c
@brief		PS2 device routines
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hwdefs.h"

#ifndef PS2_HALF_CLK
	#define PS2_HALF_CLK 20
#endif

#define PS2_CLK_HIGH (PIN(PS2_CLK_PORT) & _BV(PS2_CLK_BIT))
#define PS2_DATA_HIGH (PIN(PS2_DATA_PORT) & _BV(PS2_DATA_BIT))

/** @privatesection */

void ps2_clk(uint8_t high)
{
	if( high ) {
		PS2_CLK_PORT |= _BV(PS2_CLK_BIT);
		DDR(PS2_CLK_PORT) &= ~_BV(PS2_CLK_BIT);
	} else {
		PS2_CLK_PORT &= ~_BV(PS2_CLK_BIT);
		DDR(PS2_CLK_PORT) |= _BV(PS2_CLK_BIT);
	}
}

void ps2_data(uint8_t high)
{
	if( high ) {
		PS2_DATA_PORT |= _BV(PS2_DATA_BIT);
		DDR(PS2_DATA_PORT) &= ~_BV(PS2_DATA_BIT);
	} else {
		PS2_DATA_PORT &= ~_BV(PS2_DATA_BIT);
		DDR(PS2_DATA_PORT) |= _BV(PS2_DATA_BIT);
	}
}

/** @publicsection */

/**
@brief Init bus
*/
void ps2_init(void)
{
	ps2_clk(1);
	ps2_data(1);
}

/**
@brief Send byte
@param[in]	b	byte to send
@return Zero if ok, otherwise error code
*/
uint8_t ps2_send(uint8_t b)
{
	if( PS2_DATA_HIGH == 0 ) return 1;		// data line low, error 1

	uint8_t i = 0;

	while( PS2_CLK_HIGH == 0 ) {
		_delay_us(PS2_HALF_CLK);
		if( ++i > 30 ) return 2;			// clk line low, error 2
	}

	uint8_t par = 3;						// stop and parity bits

	for( i = 0; i < 11; i++ ) {
		if( (i > 0) && (b & 1) ) {			// set data line
			ps2_data(1);
			par ^= 1;
		} else {
			ps2_data(0);
		}
		_delay_us(PS2_HALF_CLK);			// pulse clock
		ps2_clk(0);
		_delay_us(2 * PS2_HALF_CLK);
		ps2_clk(1);
		_delay_us(PS2_HALF_CLK);

		if( i > 0 ) b >>= 1;				// don't shift data for start bit
		if( i == 8 ) b = par;				// data bits transmitted, copy stop and parity
	}

	_delay_us(4*PS2_HALF_CLK);
	return 0;
}

/**
@brief Receive byte
@param[out]	b	received byte
@return Zero if ok, otherwise error code
*/
uint8_t ps2_recv(uint8_t* b)
{
	if( PS2_DATA_HIGH != 0 ) return 1;		// data line high, error 1

	uint8_t i = 0;
	while( PS2_CLK_HIGH == 0 ) {			// if clock is low, wait a while for it to return high
		_delay_us(10);
		if( ++i == 25 ) return 2;			// timeout, error 2
	}

	uint8_t par = 1;
	uint8_t rxp = 0;
	for( i = 0; i < 11; i++ ) {				// get 8 bits, parity, generate ack
		if( i == 10 ) ps2_data(0);			// ack

		_delay_us(2*PS2_HALF_CLK);
		ps2_clk(0);
		_delay_us(2 * PS2_HALF_CLK);
		ps2_clk(1);

		if( i < 8 ) {
			*b >>= 1;
			if( PS2_DATA_HIGH ) {
				*b |= 0x80;
				par ^= 1;
			}
		}

		if( i == 8 ) {
			if( PS2_DATA_HIGH ) rxp = 1;
		}
	}

	ps2_data(1);

	if( par != rxp ) return 3;				// invalid parity, error 3

	return 0;
}
