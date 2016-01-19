/**
For this to work, you must use swi2c.c and define
#define SWI2C_RAWADR
#define SWI2C_REVBITS
in swdefs.h since someone at Tian Micro decided it would be a good idea to put bits in reverse than what i2c spec says.

Also note that on the modules I got, the i2c pullups are 10k coupled with some 1nF caps which makes it work
at very low i2c speeds only. My recommendation is remove the caps and replace the resistors with 2k.

@file		tm1637.c
@brief		TM1637 8-segment LED driver routines.
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "i2c.h"

/** @privatesection */

// XGFEDCBA
const uint8_t ascii2seg7[] PROGMEM = {
0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,	// digits
0,0,0,0,0,0,0, // colon to at sign (NI)
0x77,0x7c,0x39,0x5e,0x79,0x71,0x3d,0x74,0x30,0x1e,0x75,0x38,0x15,0x54,0x3f,0x73,0x67,0x50,0x6d,0x78,0x3e,0x1c,0x2a,0x76,0x6e,0x5b // chars
};
static uint8_t bri = 2;

void tm1637_encode(char* s, uint8_t* d, uint8_t len)
{
	uint8_t i;

	for( i = 0; i < len; ++i ) {
		if( (*s >= '0') && (*s <= 'Z') ) {
			*d = pgm_read_byte(&ascii2seg7[*s-'0']);
		} else
		if( (*s >= 'a') && (*s <= 'z') ) {
			*d = pgm_read_byte(&ascii2seg7[*s-'a'+'A'-'0']);
		} else
		if( *s == '-' ) {
			*d = 0x40;
		} else {
			*d = 0;
		}
		s++;
		d++;
	}
}

/** @publicsection */

/**
@brief Set brightness. Effective on next display update.
@param[in]	a		Brightness (0..7)
*/
void tm1637_setbri(uint8_t a)
{
	bri = a & 0x07;
}

/**
@brief Display string
@param[in]	s		String to display
@param[in]	len		String length
*/
void tm1637_putsn(char* s, uint8_t len)
{
	uint8_t d[len];
	tm1637_encode(s, d, len);

	i2c_writebuf(0x40, 0, 0);
	_delay_us(200);
	i2c_writebuf(0xc0, d, len);
	_delay_us(200);
	i2c_writebuf(0x88 + bri, 0, 0);
	_delay_us(200);
}

/**
@brief Display int
@param[in]	a		int
@param[in]	r		Radix
@param[in]	w		Min width
@param[in]	c		Prepending char to achieve min width
*/
void tm1637_puti_lc(int16_t a, uint8_t r, uint8_t w, char c)
{
	char s[10+w];
	memset(s, c, 10+w);

	ltoa(a, s+w, r);

	uint8_t l = strlen(s+w);

	if( w > l ) {
		tm1637_putsn(s+l, w);
	} else {
		tm1637_putsn(s+w, l);
	}
}
