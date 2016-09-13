#ifndef MAT_SPI_H
#define MAT_SPI_H

#include <inttypes.h>
#include <avr/io.h>

#define SPI_FDIV_4 0
#define SPI_FDIV_16 1
#define SPI_FDIV_64 2
#define SPI_FDIV_128 3
#define SPI_FDIV_2 4
#define SPI_FDIV_8 5
#define SPI_FDIV_32 6

// init SPI
void spi_init(uint8_t fdiv);

// send a byte over SPI
uint8_t spi_rw(uint8_t d);

// set SPI mode (0..3)
void spi_mode(uint8_t m);

// set SPI speed (clock divider, 0..7)
void spi_fdiv(uint8_t fdiv);

#endif
