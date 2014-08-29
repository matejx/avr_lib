// ------------------------------------------------------------------
// --- time.c                                                     ---
// --- library for time conversions                               ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

#ifndef MAT_TIME_H
#define MAT_TIME_H

#include <inttypes.h>

struct rtc_t
{
	uint8_t sec;	// 0..59
	uint8_t min;	// 0..59
	uint8_t hr;		// 0..23
	uint8_t day;	// 1..31
	uint8_t mon;	// 1..12
	uint8_t yr;		// 0..99
};

uint32_t rtc_mktime(const struct rtc_t* t);
uint8_t rtc_timevalid(const struct rtc_t* t);
void rtc_inct(struct rtc_t* t);

#endif
