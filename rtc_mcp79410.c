// ------------------------------------------------------------------
// --- rtc_mcp79410.c                                             ---
// --- library for Microchip RTC                                  ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

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

extern void fatal(PGM_P);

void rtc_error(PGM_P p)
{
#ifdef RTC_FATAL_ERR
	fatal(p); 
#endif
}

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

void rtc_stop(void)
{
	if( rtc_writebyte2(RTC_I2C_ADR, 0, 0) ) { rtc_error(PSTR("rtc_stop")); }
	_delay_ms(2);	// roughly 32 RTC clock cycles
}

// ------------------------------------------------------------------

void rtc_start(uint8_t sec)
{
	if( rtc_writebyte2(RTC_I2C_ADR, 0, rtc_dec2bcd(sec) | _BV(RTC_OSCEN)) ) { rtc_error(PSTR("rtc_start")); }
	_delay_ms(2);	// roughly 32 RTC clock cycles
}

// ------------------------------------------------------------------

uint8_t rtc_getsec(void)
{
	i2c_writebyte(RTC_I2C_ADR, 0);
	return rtc_bcd2dec(i2c_readbyte(RTC_I2C_ADR) & 0x7f);
}

// ------------------------------------------------------------------

void rtc_gettime1(struct rtc_t* t)
{
	i2c_writebyte(RTC_I2C_ADR, 0);
	uint8_t buf[7];
	i2c_readbuf(RTC_I2C_ADR, buf, sizeof(buf));
	t->sec = rtc_bcd2dec(buf[0] & 0x7f);
	t->min = rtc_bcd2dec(buf[1] & 0x7f);
	t->hr  = rtc_bcd2dec(buf[2] & 0x3f);
	t->day = rtc_bcd2dec(buf[4] & 0x3f);
	t->mon = rtc_bcd2dec(buf[5] & 0x1f);
	t->yr  = rtc_bcd2dec(buf[6]);
}

void rtc_gettime(struct rtc_t* t)
{
	rtc_gettime1(t);
	
	if( t->sec == 59 ) {
		struct rtc_t r;
		rtc_gettime1(&r);
		
		if( t->sec != r.sec ) { 
			memcpy(t, &r, sizeof(r));	
		}
	}
}

// ------------------------------------------------------------------

void rtc_settime(const struct rtc_t* t)
{
	rtc_stop();
	uint8_t buf[8];
	buf[0] = 1; // min register address
	buf[1] = rtc_dec2bcd(t->min);
	buf[2] = rtc_dec2bcd(t->hr);
	buf[3] = _BV(RTC_VBATEN) | 1;
	buf[4] = rtc_dec2bcd(t->day);
	buf[5] = rtc_dec2bcd(t->mon);
	buf[6] = rtc_dec2bcd(t->yr);
	buf[7] = 0; // control register
	if( i2c_writebuf(RTC_I2C_ADR, buf, sizeof(buf)) ) rtc_error(PSTR("rtc_settime"));
	rtc_start(t->sec);
}

// ------------------------------------------------------------------

void rtc_init(void)
{
	i2c_init(8);		// 100kHz @ 8MHz

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
