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

#define LCD_DELAY asm("nop\nnop\nnop\nnop\n")

#define LCD_BUSY_PCF_BIT 3
#define LCD_BL_PCF_BIT 7

#define LCD_RS_1 pcfBit(4, 1)
#define LCD_RS_0 pcfBit(4, 0)

#define LCD_RW_1 pcfBit(5, 1)
#define LCD_RW_0 pcfBit(5, 0)

#define LCD_E_1 pcfBit(6, 1)
#define LCD_E_0 pcfBit(6, 0)

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

void pcfBit(const uint8_t bit, const uint8_t on)
{
	if( on ) {
		pcfWrite(pcfLast | _BV(bit));
	} else {
		pcfWrite(pcfLast & ~_BV(bit));
	}
}

void lcd_out(const uint8_t data, const uint8_t rs)
{
	LCD_E_1;				// E high
	if( rs ) {LCD_RS_1;} else {LCD_RS_0;}	// select instruction(0) or data(1)
	LCD_RW_0;				// RW low (write)
	pcfWrite((pcfLast & 0xf0) | (data >> 4));		// set data pins
	LCD_DELAY;
	LCD_E_0;				// high to low on E to clock data
}

uint8_t lcd_in(const uint8_t rs)
{
	if( rs ) {LCD_RS_1;} else {LCD_RS_0;}	// select instruction/data
	LCD_RW_1;					// RW high (read)
	LCD_E_1;
	LCD_DELAY;
	uint8_t c = pcfRead() & 0x0f;
	LCD_E_0;

	return c;
}

uint8_t lcd_busy(void)
{
	pcfBit(LCD_BUSY_PCF_BIT, 1);
	return (lcd_in(0) & _BV(LCD_BUSY_PCF_BIT));
}

uint8_t lcd_available(void)	// returns 0(false) on timeout and 1-20(true) on lcd available
{
	uint8_t i = 10;			// wait max 10ms
	while ( lcd_busy() && i ) {
		i--;
	}
	return i;
}

uint8_t lcd_wr(const uint8_t d, const uint8_t rs)
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
	pcfBit(LCD_BL_PCF_BIT, on);
}

// initialize lcd interface
uint8_t lcd_hwinit(void)
{
	i2c_init(I2C_100K);

	//lcd_bl(0);		// backlight off
	pcfWrite(pcfLast & _BV(LCD_BL_PCF_BIT)); // set all zeros except BL bit
	//LCD_E_0;			// idle E is low

	return pcfErr;
}

// set pcf8574 address (if different from default 0x40)
uint8_t lcd_pcfadr(uint8_t a)
{
	if( a ) pcfAdr = a;
	return pcfAdr;
}
