#ifndef MAT_EE95_H
#define MAT_EE95_H

#include <inttypes.h>

void ee95_init(uint8_t fdiv);
void ee95_rd(uint16_t adr, uint8_t* buf, uint16_t len);
void ee95_wr(uint16_t adr, uint8_t d);
uint8_t ee95_rdsr(void);

#endif
