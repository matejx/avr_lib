#ifndef MAT_RTC_H
#define MAT_RTC_H

#include <inttypes.h>
#include "time.h"

extern const char RTC_IMPL[] PROGMEM;

void rtc_init(void);
uint8_t rtc_gettime(struct rtc_t* t);
uint8_t rtc_settime(const struct rtc_t* t);
int8_t rtc_getcal(void);
void rtc_setcal(int8_t c);
uint8_t rtc_gettemp(float* f);

#endif
