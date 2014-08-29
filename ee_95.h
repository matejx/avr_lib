#ifndef MAT_EE95_H
#define MAT_EE95_H

#include <inttypes.h>

#define EE95_CS_PORT PORTA
#define EE95_CS_BIT 2
#define EE95_WP_PORT PORTA
#define EE95_WP_BIT 3

void ee95_init(void);
void ee95_rd(uint16_t adr, uint8_t* buf, uint16_t len);
void ee95_wr(uint16_t adr, uint8_t d);
uint8_t ee95_rdsr(void);

#endif
