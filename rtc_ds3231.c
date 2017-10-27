/**
@file		rtc_ds3231.c
@brief		RTC implementation with DS3231
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

const char RTC_IMPL[] PROGMEM = "DS3";

#define RTC_I2C_ADR 0xd0

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

// ------------------------------------------------------------------

uint8_t rtc_writebyte2(const uint8_t adr, const uint8_t data1, const uint8_t data2)
{
	uint8_t data[2];
	data[0] = data1;
	data[1] = data2;
	return i2c_writebuf(adr, data, 2);
}

// ------------------------------------------------------------------

uint8_t rtc_gettime(struct rtc_t* t)
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

// ------------------------------------------------------------------

uint8_t rtc_settime(const struct rtc_t* t)
{
	uint8_t buf[7];
	buf[0] = 0; // min register address
	buf[1] = rtc_dec2bcd(t->sec);
	buf[2] = rtc_dec2bcd(t->min);
	buf[3] = rtc_dec2bcd(t->hr);
	buf[4] = rtc_dec2bcd(t->day);
	buf[5] = rtc_dec2bcd(t->mon);
	buf[6] = rtc_dec2bcd(t->yr);
	if( i2c_writebuf(RTC_I2C_ADR, buf, sizeof(buf)) ) return 1;
	return 0;
}

// ------------------------------------------------------------------

void rtc_init(void)
{
	i2c_init(I2C_100K);
}

// ------------------------------------------------------------------

int8_t rtc_getcal(void)
{
	return 0;
}

// ------------------------------------------------------------------

void rtc_setcal(int8_t c)
{
}

// ------------------------------------------------------------------

uint8_t rtc_gettemp(float* f)
{
	if( i2c_writebyte(RTC_I2C_ADR, 0x11) ) return 1;
	uint8_t buf[2];
	if( i2c_readbuf(RTC_I2C_ADR, buf, sizeof(buf)) ) return 1;
	*f = (int8_t)buf[0];
	if( buf[1] & 0x80 ) *f += 0.50;
	if( buf[1] & 0x40 ) *f += 0.25;
	return 0;
}

// ------------------------------------------------------------------
