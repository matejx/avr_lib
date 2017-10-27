/**
@file		rtc_mcp79410.c
@brief		RTC implementation with MCP79410
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "time.h"
#include "i2c.h"

const char RTC_IMPL[] PROGMEM = "MCP";

#define RTC_I2C_ADR 0xde
#define RTC_VBATEN 3
#define RTC_OSCEN 7
#define RTC_24H 6

// ------------------------------------------------------------------

uint8_t rtc_bcd2dec(const uint8_t b)
{
	return ((b >> 4) * 10) + (b & 0x0f);
}

// ------------------------------------------------------------------

uint8_t rtc_dec2bcd(const uint8_t b)
{
	uint8_t d = b / 10;

	return (d << 4) | (b % 10);
}

uint8_t rtc_writebyte2(const uint8_t adr, const uint8_t data1, const uint8_t data2)
{
	uint8_t data[2];
	data[0] = data1;
	data[1] = data2;
	return i2c_writebuf(adr, data, 2);
}

// ------------------------------------------------------------------
// invalidates time, leaves internal register counter at 1 (minutes)

uint8_t rtc_stop(void)
{
	if( rtc_writebyte2(RTC_I2C_ADR, 0, 0) ) return 1;
	_delay_ms(2);	// roughly 32 RTC clock cycles
	return 0;
}

// ------------------------------------------------------------------

uint8_t rtc_start(uint8_t sec)
{
	if( rtc_writebyte2(RTC_I2C_ADR, 0, rtc_dec2bcd(sec) | _BV(RTC_OSCEN)) ) return 1;
	_delay_ms(2);	// roughly 32 RTC clock cycles
	return 0;
}

// ------------------------------------------------------------------

uint8_t rtc_getsec(void)
{
	i2c_writebyte(RTC_I2C_ADR, 0);
	return rtc_bcd2dec(i2c_readbyte(RTC_I2C_ADR) & 0x7f);
}

// ------------------------------------------------------------------

uint8_t rtc_gettime1(struct rtc_t* t)
{
	if( i2c_writebyte(RTC_I2C_ADR, 0) ) return 1;
	uint8_t buf[7];
	if( i2c_readbuf(RTC_I2C_ADR, buf, sizeof(buf)) ) return 1;
	t->sec = rtc_bcd2dec(buf[0] & 0x7f);
	t->min = rtc_bcd2dec(buf[1] & 0x7f);
	t->hr  = rtc_bcd2dec(buf[2] & 0x3f);
	t->day = rtc_bcd2dec(buf[4] & 0x3f);
	t->mon = rtc_bcd2dec(buf[5] & 0x1f);
	t->yr  = rtc_bcd2dec(buf[6]);
	return 0;
}

uint8_t rtc_gettime(struct rtc_t* t)
{
	if( rtc_gettime1(t) ) return 1;

	if( t->sec == 59 ) {
		struct rtc_t r;
		if( rtc_gettime1(&r) ) return 1;

		if( t->sec != r.sec ) {
			memcpy(t, &r, sizeof(r));
		}
	}

	return 0;
}

// ------------------------------------------------------------------

uint8_t rtc_settime(const struct rtc_t* t)
{
	if( rtc_stop() ) return 1;
	uint8_t buf[8];
	buf[0] = 1; // min register address
	buf[1] = rtc_dec2bcd(t->min);
	buf[2] = rtc_dec2bcd(t->hr);
	buf[3] = _BV(RTC_VBATEN) | 1;
	buf[4] = rtc_dec2bcd(t->day);
	buf[5] = rtc_dec2bcd(t->mon);
	buf[6] = rtc_dec2bcd(t->yr);
	buf[7] = 0; // control register
	if( i2c_writebuf(RTC_I2C_ADR, buf, sizeof(buf)) ) return 1;
	if( rtc_start(t->sec) ) return 1;
	return 0;
}

// ------------------------------------------------------------------

void rtc_init(void)
{
	i2c_init(I2C_100K);

	struct rtc_t t;
	rtc_gettime(&t);
	rtc_settime(&t);
}

// ------------------------------------------------------------------

int8_t rtc_getcal(void)
{
	i2c_writebyte(RTC_I2C_ADR, 8);
	uint8_t c = i2c_readbyte(RTC_I2C_ADR);
	int8_t r = c & 0x7f;

	if( c & 0x80 ) return -r;
	return r;
}

// ------------------------------------------------------------------

void rtc_setcal(int8_t c)
{
	if( c == 128 ) c = 127;
	uint8_t r = (c >= 0) ? c : -c;
	if( c < 0 ) r |= 0x80;

	rtc_writebyte2(RTC_I2C_ADR, 8, r);
}

// ------------------------------------------------------------------
