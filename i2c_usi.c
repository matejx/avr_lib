
#include <avr/io.h>
#include <avr/interrupt.h>
#include "circbuf8.h"
#include "hwdefs.h"

#define USI_SLAVE_CHECK_ADDRESS                (0x00)
#define USI_SLAVE_SEND_DATA                    (0x01)
#define USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA (0x02)
#define USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA   (0x03)
#define USI_SLAVE_RECV_DATA                 (0x04)
#define USI_SLAVE_GET_DATA_AND_SEND_ACK        (0x05)

void SET_USI_TO_SEND_ACK(void)
{
	USIDR =  0; // Prepare ACK
	DDR(PORT_USI) |= _BV(PORT_USI_SDA); // Set SDA as output
	USISR =  _BV(USIOIF)|_BV(USIPF)|_BV(USIDC)|(0x0e<<USICNT0); // Clear all flags, except Start Cond, set USI counter to shift 1 bit.
}

void SET_USI_TO_READ_ACK(void)
{
	DDR(PORT_USI) &= ~_BV(PORT_USI_SDA); // Set SDA as intput
	USIDR = 0; // Prepare ACK
	USISR = _BV(USIOIF)|_BV(USIPF)|_BV(USIDC)|(0x0e<<USICNT0); // Clear all flags, except Start Cond, set USI counter to shift 1 bit.
}

void SET_USI_TO_WAIT_FOR_START(void)
{
	// Enable Start Condition Interrupt. Disable Overflow Interrupt. Set USI in Two-wire mode. No USI Counter overflow hold. Shift Register Clock Source = External, positive edge
	USICR = _BV(USISIE)|_BV(USIWM1)|_BV(USICS1);
	USISR = _BV(USIOIF)|_BV(USIPF)|_BV(USIDC); // Clear all flags, except Start Cond
}

void SET_USI_TO_SEND_DATA(void)
{
	DDR(PORT_USI) |= _BV(PORT_USI_SDA); // Set SDA as output
	USISR = _BV(USIOIF)|_BV(USIPF)|_BV(USIDC); // Clear all flags, except Start Cond, set USI to shift out 8 bits
}

void SET_USI_TO_READ_DATA(void)
{
	DDR(PORT_USI) &= ~_BV(PORT_USI_SDA); // Set SDA as input
	USISR = _BV(USIOIF)|_BV(USIPF)|_BV(USIDC); // Clear all flags, except Start Cond, set USI to shift out 8 bits
}

static uint8_t TWI_slaveAddress;
static volatile uint8_t USI_TWI_Overflow_State;

static volatile struct cbuf8_t i2c_rxq;
static volatile struct cbuf8_t i2c_txq;

void i2c_usi_init(uint8_t addr, uint8_t* txb, uint8_t txs, uint8_t* rxb, uint8_t rxs)
{
	cbuf8_clear(&i2c_txq, txb, txs);
	cbuf8_clear(&i2c_rxq, rxb, rxs);

	TWI_slaveAddress = addr;

	PORT_USI |= _BV(PORT_USI_SCL); // Set SCL high
	PORT_USI |= _BV(PORT_USI_SDA); // Set SDA high
	DDR(PORT_USI) |=  _BV(PORT_USI_SCL); // Set SCL as output
	DDR(PORT_USI) &= ~_BV(PORT_USI_SDA); // Set SDA as input
	// Enable Start Condition Interrupt. Disable Overflow Interrupt.
	// Set USI in Two-wire mode. No USI Counter overflow prior to first Start Condition (potentail failure)
	// Shift Register Clock Source = External, positive edge
	USICR = _BV(USISIE)|_BV(USIWM1)|_BV(USICS1);
	USISR = 0xf0; // Clear all flags and reset overflow counter
}

uint8_t i2c_usi_getc(uint8_t* const d)
{
	return cbuf8_get(&i2c_rxq, d);
}

uint8_t i2c_usi_putc(const uint8_t d)
{
	return cbuf8_put(&i2c_rxq, d);
}

// USI start condition ISR. Detects the USI_TWI Start Condition and intialises the USI for reception of the "TWI Address" packet.
ISR(USI_START_vect)
{
    uint8_t d = USISR; // Temporary variable to store volatile

	// Set default starting conditions for new TWI packet
    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
    DDR(PORT_USI)  &= ~_BV(PORT_USI_SDA); // Set SDA as input

	// Wait for SCL to go low to ensure the "Start Condition" has completed. If a Stop condition arises then leave the interrupt to prevent waiting forever.
    while ( (PIN(PORT_USI) & _BV(PORT_USI_SCL)) & !(d & _BV(USIPF)) );

	// Enable Overflow and Start Condition Interrupt. (Keep StartCondInt to detect RESTART)
    USICR = _BV(USISIE)|_BV(USIOIE)|_BV(USIWM1)|_BV(USIWM0)|_BV(USICS1);

	// Clear flags. Set USI to sample 8 bits i.e. count 16 external pin toggles.
    USISR = _BV(USISIF)|_BV(USIOIF)|_BV(USIPF)|_BV(USIDC);
}

// USI counter overflow ISR. Handles all the comunication. Is disabled only when waiting for new Start Condition.
ISR(USI_OVERFLOW_vect)
{
	uint8_t d;

	switch( USI_TWI_Overflow_State )
	{
		// ---------- Address mode ----------
		// Check address and send ACK (and next USI_SLAVE_SEND_DATA) if OK, else reset USI.
		case USI_SLAVE_CHECK_ADDRESS:
			if( (USIDR == 0) || (( USIDR>>1 ) == TWI_slaveAddress) ) {
				if( USIDR & 0x01 ) {
				  USI_TWI_Overflow_State = USI_SLAVE_SEND_DATA;
				} else {
				  USI_TWI_Overflow_State = USI_SLAVE_RECV_DATA;
				}
				SET_USI_TO_SEND_ACK();
			} else {
				SET_USI_TO_WAIT_FOR_START();
			}
			break;

		// ----- Master write data mode ------
		// Check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI.
		case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
			if( USIDR ) { // If NACK, the master does not want more data.
				SET_USI_TO_WAIT_FOR_START();
				return;
			}
			// From here we just drop straight into USI_SLAVE_SEND_DATA if the master sent an ACK

		// Copy data from buffer to USIDR and set USI to shift byte. Next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
		case USI_SLAVE_SEND_DATA:
			if( cbuf8_get(&i2c_txq, &d) ) {
				USIDR = d;
			} else {
				SET_USI_TO_WAIT_FOR_START();
				return;
			}
			USI_TWI_Overflow_State = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
			SET_USI_TO_SEND_DATA();
			break;

		// Set USI to sample reply from master. Next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
		case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
			USI_TWI_Overflow_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
			SET_USI_TO_READ_ACK();
			break;

		// ----- Master read data mode ------
		// Set USI to sample data from master. Next USI_SLAVE_GET_DATA_AND_SEND_ACK.
		case USI_SLAVE_RECV_DATA:
			USI_TWI_Overflow_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;
			SET_USI_TO_READ_DATA();
			break;

		// Copy data from USIDR and send ACK. Next USI_SLAVE_RECV_DATA
		case USI_SLAVE_GET_DATA_AND_SEND_ACK:
			d = USIDR;
			cbuf8_put(&i2c_rxq, d);
			USI_TWI_Overflow_State = USI_SLAVE_RECV_DATA;
			SET_USI_TO_SEND_ACK();
			break;
	}
}
