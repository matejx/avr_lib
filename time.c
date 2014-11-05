// ------------------------------------------------------------------
// --- time.c                                                     ---
// --- library for time conversions                               ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

#include "time.h"

const uint32_t MINUTE = 60;
const uint32_t HOUR = 3600;
const uint32_t DAY  = 86400;
const uint32_t YEAR = 31536000;

static uint16_t month[12] = {
	0,
	(31),
	(31+28),
	(31+28+31),
	(31+28+31+30),
	(31+28+31+30+31),
	(31+28+31+30+31+30),
	(31+28+31+30+31+30+31),
	(31+28+31+30+31+30+31+31),
	(31+28+31+30+31+30+31+31+30),
	(31+28+31+30+31+30+31+31+30+31),
	(31+28+31+30+31+30+31+31+30+31+30)
};

// ------------------------------------------------------------------

uint32_t rtc_mktime(const struct rtc_t* t)
{
	uint32_t r =  946684800;  // 1.1.2000 0:0:0 GMT
//	r -= 3600;	// CET (GMT-1)

	r += (t->yr * YEAR);
	if( t->yr ) {
		r += DAY;       // 2000 (00) was a leap year
		r += ((t->yr-1) / 4) * DAY;
	}

	r += month[t->mon-1] * DAY;
	if( (t->mon > 2) && (0 == (t->yr % 4)) )
		r += DAY;

	r += (t->day-1) * DAY;
	r += t->hr * HOUR;
	r += t->min * MINUTE;
	r += t->sec;

	return r;
}

// ------------------------------------------------------------------

uint8_t rtc_timevalid(const struct rtc_t* t)
{
	return (
		(t->yr >= 14)&&(t->yr <= 29)&&
		(t->mon >= 1)&&(t->mon <= 12)&&
		(t->day >= 1)&&(t->day <= 31)&&
		(t->hr <= 23)&&(t->min <= 59)&&(t->sec <= 59)
		);
}

// ------------------------------------------------------------------

const uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void rtc_inct(struct rtc_t* t)
{
	if( ++t->sec > 59 ) {
		t->sec = 0;
		if( ++t->min > 59 ) {
			t->min = 0;
			if( ++t->hr > 23 ) {
				t->hr = 0;
				uint8_t leapmon = (t->mon == 2) && (t->yr % 4 == 0);
				if( ++t->day > month_days[t->mon-1] + leapmon ) {
					t->day = 1;
					if( ++t->mon > 12 ) {
						t->mon = 1;
						++t->yr;
					}
				}
			}
		}
	}
}
