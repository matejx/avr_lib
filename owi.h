#ifndef _OWI_ROM_FUNCTIONS_H_
#define _OWI_ROM_FUNCTIONS_H_

#include <inttypes.h>

void owi_init(uint8_t pins);

uint8_t owi_detect(uint8_t pins);
void owi_skip(uint8_t pins);
void owi_read(uint8_t* romValue, uint8_t pins);
void owi_match(uint8_t* romValue, uint8_t pins);

uint8_t owi_crcok(uint8_t* romValue);

#endif
