#ifndef MAT_PS2_H
#define MAT_PS2_H

#include <inttypes.h>

void ps2_init(void);
uint8_t ps2_send(uint8_t b);
uint8_t ps2_recv(uint8_t* b);

#endif
