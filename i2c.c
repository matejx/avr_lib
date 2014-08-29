// ------------------------------------------------------------------
// --- i2c.c                                                      ---
// --- library for hardware i2c communication                     ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>
#include <util/twi.h>

#include "swdefs.h"

#ifdef I2C_USE_CMT
	#warning I2C using cmt
	#include "cmt.h"
	struct cmt_mutex i2c_mutex;
#endif

#define I2C_RETRIES 10

#define I2C_START 0
#define I2C_STOP 1
#define I2C_DATA_TX 2
#define I2C_DATA_RX_NACK 3
#define I2C_DATA_RX_ACK 4

uint8_t i2cTransfer(uint8_t cmd, uint8_t data)
{
	if( cmd == I2C_DATA_TX ) { TWDR = data; }

	uint8_t c = _BV(TWINT) | _BV(TWEN);
	if( cmd == I2C_START       ) c |= _BV(TWSTA);
	if( cmd == I2C_STOP        ) c |= _BV(TWSTO);
	if( cmd == I2C_DATA_RX_ACK ) c |= _BV(TWEA);
	TWCR = c;

	if( cmd != I2C_STOP ) {
		while( !(TWCR & _BV(TWINT)) ) {
			#ifdef I2C_USE_CMT
			cmt_delay_ticks(0);
			#endif
		}
	}

	return TWDR;
}

// ------------------------------------------------------------------
// public functions
// ------------------------------------------------------------------

void i2c_init(uint8_t br)
{
	if( TWCR & _BV(TWEN) ) return;

	#ifdef I2C_USE_CMT
	i2c_mutex.ac = 0;
	#endif

	TWBR = br;
}

/*
uint8_t i2c_present(uint8_t adr)
{
	i2cTransfer(I2C_START, 0); // always successful (waits for bus)
	i2cTransfer(I2C_DATA_TX, adr | 1); // slave NACK or lost arb
	uint8_t r = (TW_STATUS == TW_MT_SLA_ACK);
	if( TW_STATUS != TW_MT_ARB_LOST ) i2cTransfer(I2C_STOP, 0);
	return r;
}
*/

uint8_t i2c_writebuf(const uint8_t adr, uint8_t* const data, const uint8_t len)
{
	#ifdef I2C_USE_CMT
	cmt_acquire(&i2c_mutex);
	#endif

	uint8_t retries = I2C_RETRIES;
	uint8_t r;

	while( retries-- ) {
		i2cTransfer(I2C_START, 0); // always successful (waits for bus)

		i2cTransfer(I2C_DATA_TX, adr & 0xfe); // slave NACK or lost arb
		if( (r = TW_STATUS == TW_MT_SLA_NACK) ) break;	// slave not present
		if( TW_STATUS != TW_MT_SLA_ACK ) continue; // some other error, retry

		uint8_t l = len;
		uint8_t* d = data;
		while( l ) {
			i2cTransfer(I2C_DATA_TX, *d);	// data NACK or lost arb
			if( TW_STATUS != TW_MT_DATA_ACK ) break; // retry in any case
			l--;
			d++;
		}
		if( l ) continue; // break in preceding while loop - continue (retry)

		break;
	}

	if( TW_STATUS != TW_MT_ARB_LOST ) {
		i2cTransfer(I2C_STOP, 0);
	}

	#ifdef I2C_USE_CMT
	cmt_release(&i2c_mutex);
	#endif

	return r;
}

uint8_t i2c_readbuf(const uint8_t adr, uint8_t* const data, const uint8_t len)
{
	#ifdef I2C_USE_CMT
	cmt_acquire(&i2c_mutex);
	#endif

	uint8_t retries = I2C_RETRIES;
	uint8_t r;

	while( retries-- ) {
		i2cTransfer(I2C_START, 0); // always successful (waits for bus)

		i2cTransfer(I2C_DATA_TX, adr | 1);	// slave NACK or lost arb
		if( (r = TW_STATUS == TW_MR_SLA_NACK) ) break; // slave not present
		if( TW_STATUS != TW_MR_SLA_ACK ) continue; // some other error, retry

		uint8_t l = len;
		uint8_t* d = data;
		while( l > 1 ) {
			*d = i2cTransfer(I2C_DATA_RX_ACK, 0); // lost arb
			if( TW_STATUS != TW_MR_DATA_ACK ) break;	// retry in any case
			l--;
			d++;
		}
		if( l > 1 ) continue; // break in preceding while loop - continue (retry)
		*d = i2cTransfer(I2C_DATA_RX_NACK, 0); // always successful

		break;
	}

	if( TW_STATUS != TW_MR_ARB_LOST ) {
		i2cTransfer(I2C_STOP, 0);
	}

	#ifdef I2C_USE_CMT
	cmt_release(&i2c_mutex);
	#endif

	return r;
}
