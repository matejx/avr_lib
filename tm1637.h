#ifndef MAT_TM1637_H
#define MAT_TM1637_H

#include <inttypes.h>

void tm1637_setbri(uint8_t a);
void tm1637_putsn(char* s, uint8_t len);
void tm1637_puti_lc(int16_t a, uint8_t r, uint8_t w, char c);
#define tm1637_puti(a,b) tm1637_puti_lc(a,b,4,' ')

#endif
