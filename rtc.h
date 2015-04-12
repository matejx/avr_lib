#ifndef MAT_RTC_H
#define MAT_RTC_H

#include <inttypes.h>
#include "time.h"

extern const char RTC_IMPL[] PROGMEM;

void rtc_init(void);
void rtc_gettime(struct rtc_t* t);
void rtc_settime(const struct rtc_t* t);
int8_t rtc_getcal(void);
void rtc_setcal(int8_t c);

#endif
