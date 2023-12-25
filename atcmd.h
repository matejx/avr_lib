#ifndef MAT_ATCMD_H
#define MAT_ATCMD_H

#include <inttypes.h>

uint8_t atc_wait_reply(const uint8_t n, const PGM_P reply, const uint16_t to_ms);
uint8_t atc_at_cmd(const uint8_t n, const PGM_P cmd1, const char* cmd2, const PGM_P reply, const uint16_t to_ms);

#endif
