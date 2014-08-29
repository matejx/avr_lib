#ifndef MAT_I2C_USI_H
#define MAT_I2C_USI_H

#include <inttypes.h>

void i2c_usi_init(uint8_t addr, uint8_t* txb, uint8_t txs, uint8_t* rxb, uint8_t rxs);
uint8_t i2c_usi_getc(uint8_t* const d);
uint8_t i2c_usi_putc(const uint8_t d);

#endif