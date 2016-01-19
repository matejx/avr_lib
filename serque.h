#ifndef MAT_SERIALQ_H
#define MAT_SERIALQ_H

#if F_CPU == 1000000
	#define BAUD_4800 12
#endif

#if F_CPU == 3686400
	#define BAUD_9600 23
	#define BAUD_19200 11
	#define BAUD_115200 1
#endif

#if F_CPU == 7372800
	#define BAUD_9600 47
	#define BAUD_19200 23
	#define BAUD_115200 3
#endif

#if F_CPU == 8000000
	#define BAUD_4800 103
	#define BAUD_9600 51
	#define BAUD_19200 25
	#define BAUD_38400 12
#endif

#if F_CPU == 11059200
	#define BAUD_9600 71
	#define BAUD_19200 35
	#define BAUD_115200 5
#endif

#if F_CPU == 16000000
	#define BAUD_9600 103
	#define BAUD_19200 51
	#define BAUD_38400 25
#endif

#include <inttypes.h>
#include <avr/pgmspace.h>

#define ser_puti(par1,par2,par3) ser_puti_lc(par1,par2,par3,0,0)

void ser_init(const uint8_t n, const uint16_t br, uint8_t* txb, uint8_t txs, uint8_t* rxb, uint8_t rxs);
void ser_shutdown(const uint8_t n);
void ser_flush_rxbuf(const uint8_t n);
uint8_t ser_putc(const uint8_t n, const char a);
uint8_t ser_getc(const uint8_t n, uint8_t* const d);
void ser_puts_P(const uint8_t n, const PGM_P s);
void ser_puts_esc(const uint8_t n, const char* s);
void ser_puts(const uint8_t n, const char* s);
void ser_puti_lc(const uint8_t n, const uint32_t a, const uint8_t r, uint8_t l, char c);
void ser_putf(const uint8_t n, float f, uint8_t prec);
uint8_t ser_txdone(const uint8_t n);

#endif
