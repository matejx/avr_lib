/**

RTC implementation with 32kHz crystal on Timer2 pins. Can also be used
with main crystal, but you need to calculate USEC_PER_TICK.

@file		rtc_timer2.c
@brief		RTC implementation with AVR async Timer2
@author		Matej Kogovsek
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "time.h"

const char RTC_IMPL[] PROGMEM = "TMR";

static struct rtc_t sw_time;
static int8_t sw_cal;
static uint32_t tmr_usec;

#define RTC_XTAL

#ifdef RTC_XTAL
	#define USEC_PER_TICK 15625
#else
	#warning RTC using CPU clock
//	#define USEC_PER_TICK 8000
#endif

// ------------------------------------------------------------------

void rtc_start(void)
{
#ifdef RTC_XTAL
	ASSR = _BV(AS2);
	OCR2A = 16-1;
	TCNT2 = 0;
	TCCR2A = _BV(WGM21);		// CTC operation
	TCCR2B = _BV(CS21) | _BV(CS20);	// prescale clk/32
	while( ASSR & 0x1f );
#else
	OCR2A = 250-1;
	TCNT2 = 0;
	TCCR2A = _BV(WGM21);		// CTC operation
	TCCR2B = _BV(CS22) | _BV(CS21);	// prescale clk/256
#endif
	TIFR2 = 0xff;	// clear interrupt flags
	TIMSK2 |= _BV(OCIE2A);
}

// ------------------------------------------------------------------

void rtc_stop(void)
{
	TCCR2B = 0;
#ifdef RTC_XTAL
	while( ASSR & 0x1f );
#endif
	TIMSK2 = 0;
}

// ------------------------------------------------------------------

void rtc_init(void)
{
	rtc_stop();

	memset(&sw_time, 0, sizeof(sw_time));
	sw_cal = 0;

	rtc_start();
}

// ------------------------------------------------------------------

uint8_t rtc_getsec(void)
{
	return sw_time.sec;
}

// ------------------------------------------------------------------

uint8_t rtc_gettime(struct rtc_t* t)
{
	uint8_t g = SREG;
	cli();
	memcpy(t, &sw_time, sizeof(sw_time));
	SREG = g;
	return 0;
}

// ------------------------------------------------------------------

uint8_t rtc_settime(const struct rtc_t* t)
{
	// month_days lookup table guard
	if( t->mon < 1 || t->mon > 12 ) return 1;

	uint8_t g = SREG;
	cli();
	tmr_usec = 0;
	memcpy(&sw_time, t, sizeof(sw_time));
	SREG = g;

	if( 0 == TCCR2B ) { rtc_start(); }

	return 0;
}

// ------------------------------------------------------------------

int8_t rtc_getcal(void)
{
	return sw_cal;
}

// ------------------------------------------------------------------

void rtc_setcal(int8_t c)
{
	sw_cal = c;
}

// ------------------------------------------------------------------

ISR(TIMER2_COMPA_vect)
{
	tmr_usec += USEC_PER_TICK;

	if( tmr_usec >= 1000000 ) {
		tmr_usec -= sw_cal;
	}

	if( tmr_usec >= 1000000 ) {
		tmr_usec -= 1000000;
		rtc_inct(&sw_time);
	}
}
