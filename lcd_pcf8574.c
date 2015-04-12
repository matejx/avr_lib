/**

HD44780 low level driver using PCF8574 over I2C. Define pin mapping in hwdefs.h.

@file		lcd_pcf8574.c
@brief		HD44780 lcd driver via PCF8574
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>

#include "hwdefs.h"
#include "swdefs.h"
#include "i2c.h"

// ------------------------------------------------------------------
// --- defines ------------------------------------------------------
// ------------------------------------------------------------------

const uint8_t lcd_busw = 0;

#ifndef PCF_D4
#warning Using default PCF LCD pin mapping
#define PCF_D4 _BV(0)
#define PCF_D5 _BV(1)
#define PCF_D6 _BV(2)
#define PCF_D7 _BV(3)
#define PCF_RS _BV(4)
#define PCF_RW _BV(5)
#define PCF_EN _BV(6)
#define PCF_BL _BV(7)
#endif

// ------------------------------------------------------------------
// --- private procedures -------------------------------------------
// ------------------------------------------------------------------

static uint8_t pcfBlb;
static uint8_t pcfErr = 1;
static uint8_t pcfAdr = 0x40;

void lcd_out(uint8_t data, uint8_t rs)
{
/*
	uint8_t d = 0;
	if( data & 0x80 ) d |= PCF_D7;
	if( data & 0x40 ) d |= PCF_D6;
	if( data & 0x20 ) d |= PCF_D5;
	if( data & 0x10 ) d |= PCF_D4;
*/
	if( rs ) rs = PCF_RS;
	uint8_t d[2];
	d[1] = pcfBlb | rs | (data >> 4);
	d[0] = d[1] | PCF_EN;
	pcfErr = i2c_writebuf(pcfAdr , d, 2);
}

uint8_t lcd_busy(void)
{
	uint8_t r;
	uint8_t d[3];
	d[0] = pcfBlb | PCF_RW | PCF_D4 | PCF_D5 | PCF_D6 | PCF_D7;
	d[1] = d[0] | PCF_EN;
	d[2] = d[0];
	i2c_writebuf(pcfAdr , d, 2);
	i2c_readbuf(pcfAdr, &r, 1);
	pcfErr = i2c_writebuf(pcfAdr , d, 3);
	return r & PCF_D7;
}

uint8_t lcd_available(void)	// returns 0(false) on timeout and 1-20(true) on lcd available
{
	uint8_t i = 10;			// wait max 10ms
	while ( lcd_busy() && i ) { // lcd_busy takes cca. 1.2ms on 100kHz i2c bus
		--i;
	}
	return i;
}

uint8_t lcd_wr(uint8_t d, uint8_t rs)
{
	if( pcfErr ) return 0;

	if( lcd_available() ) {
		lcd_out(d, rs);
		lcd_out(d << 4, rs);
		return 1;
	}
	return 0;
}

// lcd backlight
void lcd_bl(uint8_t on)
{
	pcfBlb = on ? PCF_BL : 0;
	pcfErr = i2c_writebuf(pcfAdr , &pcfBlb, 1);
}

// initialize lcd interface
uint8_t lcd_hwinit(uint8_t p1)
{
#ifdef LCD_I2C_SPEED
	i2c_init(LCD_I2C_SPEED);
#else
	#warning LCD using I2C_100K
	i2c_init(I2C_100K);
#endif

	pcfAdr = p1;
	return i2c_writebuf(pcfAdr , &pcfBlb, 1); // set all zeros except BL bit
}

// set pcf8574 address (if different from default 0x40)
uint8_t lcd_pcfadr(void)
{
	return pcfAdr;
}
