/**
@file		time.c
@brief		Time functions
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include "time.h"

const uint32_t MINUTE = 60; /**< seconds in minute */
const uint32_t HOUR = 3600; /**< seconds in hour */
const uint32_t DAY  = 86400; /**< seconds in day */
const uint32_t YEAR = 31536000; /**< seconds in day */

static uint16_t month[12] = { /**< month of day */
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

/**
@brief Convert rtc_t to unix time.
@param[in]	t		Pointer to rtc_t
@return Unix time
*/
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

uint8_t rtc_timevalid(const struct rtc_t* t)
{
	return (
		(t->yr >= 14)&&(t->yr <= 29)&&
		(t->mon >= 1)&&(t->mon <= 12)&&
		(t->day >= 1)&&(t->day <= 31)&&
		(t->hr <= 23)&&(t->min <= 59)&&(t->sec <= 59)
		);
}

const uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /**< month length */

/**
@brief Increase rtc_t by 1 second
@param[in]	t		Pointer to rtc_t
*/
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
