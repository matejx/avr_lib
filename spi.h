// ------------------------------------------------------------------
// --- spi.c - Basic routines for sending data over SPI           ---
// ---                                                            ---
// ---                                22.dec.2009, Matej Kogovsek ---
// ------------------------------------------------------------------

#ifndef MAT_SPI_H
#define MAT_SPI_H

#include <inttypes.h>
#include <avr/io.h>

// init SPI
void spi_init(uint8_t fdiv);

// send a byte over SPI
uint8_t spi_write(uint8_t d);

#endif
