#ifndef MAT_I2C_H
#define MAT_I2C_H

#include <inttypes.h>

#if F_CPU == 8000000
	#define I2C_100K 32
	#define I2C_50K 72
#endif

#if F_CPU == 14745600
	#define I2C_100K 66
#endif

#if F_CPU == 16000000
	#define I2C_100K 72
#endif

void i2c_init(uint8_t br);
void i2c_shutdown(void);
uint8_t i2c_readbuf(const uint8_t adr, uint8_t* const data, const uint8_t len);
uint8_t i2c_writebuf(const uint8_t adr, uint8_t* const data, const uint8_t len);

// convenience functions
inline uint8_t i2c_writebyte(const uint8_t adr, uint8_t data) { return i2c_writebuf(adr, &data, 1); }
inline uint8_t i2c_readbyte(const uint8_t adr) { uint8_t r;	i2c_readbuf(adr, &r, 1); return r; }

#endif
