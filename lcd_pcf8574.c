// ------------------------------------------------------------------
// --- lcd_pcf8574.c                                              ---
// --- library for controlling the HD44780 via PCF8574            ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>

#include "hwdefs.h"
#include "i2c.h"

// ------------------------------------------------------------------
// --- defines ------------------------------------------------------
// ------------------------------------------------------------------

const uint8_t lcd_busw = 0;

#define PCF_D4 _BV(0)
#define PCF_D5 _BV(1)
#define PCF_D6 _BV(2)
#define PCF_D7 _BV(3)
#define PCF_RS _BV(4)
#define PCF_RW _BV(5)
#define PCF_EN _BV(6)
#define PCF_BL _BV(7)

// ------------------------------------------------------------------
// --- private procedures -------------------------------------------
// ------------------------------------------------------------------

static uint8_t pcfLast;
static uint8_t pcfErr = 1;
static uint8_t pcfAdr = 0x40;

void pcfWrite(uint8_t d)
{
	pcfErr = i2c_writebuf(pcfAdr , &d, 1);
	pcfLast = d;
}

uint8_t pcfRead(void)
{
	uint8_t r;
	pcfErr = i2c_readbuf(pcfAdr, &r, 1);
	return r;
}

void pcfBits(uint8_t d, uint8_t on)
{
	if( on ) {
		pcfWrite(pcfLast | d);
	} else {
		pcfWrite(pcfLast & ~d);
	}
}

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
	pcfWrite((pcfLast & PCF_BL) | PCF_EN | rs | (data >> 4));
	pcfBits(PCF_EN, 0);
}

uint8_t lcd_busy(void)
{
	pcfWrite((pcfLast & PCF_BL) | PCF_RW | PCF_D4 | PCF_D5 | PCF_D6 | PCF_D7);
	pcfBits(PCF_EN, 1);
	uint8_t r = pcfRead() & PCF_D7;
	pcfBits(PCF_EN, 0);
	pcfBits(PCF_EN, 1);
	pcfBits(PCF_EN, 0);
	return r;
}

uint8_t lcd_available(void)	// returns 0(false) on timeout and 1-20(true) on lcd available
{
	uint8_t i = 10;			// wait max 10ms
	while ( lcd_busy() && i ) { // lcd_busy takes cca. 1.2ms on 100kHz i2c bus
		i--;
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
	pcfBits(PCF_BL, on);
}

// initialize lcd interface
uint8_t lcd_hwinit(void)
{
	i2c_init(I2C_100K);

	//lcd_bl(0);		// backlight off
	pcfWrite(pcfLast & PCF_BL); // set all zeros except BL bit
	//LCD_E_0;			// idle E is low

	return pcfErr;
}

// set pcf8574 address (if different from default 0x40)
uint8_t lcd_pcfadr(uint8_t a)
{
	if( a ) pcfAdr = a;
	return pcfAdr;
}
