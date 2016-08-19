/**
@file		owi.c
@brief		1-wire interface routines
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
@note		This file was not written by me from scratch. It was adapted from AVR318 application note, http://www.atmel.com/Images/AVR318.zip
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "hwdefs.h"

#define     OWI_DELAY_A_STD_MODE    _delay_us(6)
#define     OWI_DELAY_B_STD_MODE    _delay_us(64)
#define     OWI_DELAY_C_STD_MODE    _delay_us(60)
#define     OWI_DELAY_D_STD_MODE    _delay_us(10)
#define     OWI_DELAY_E_STD_MODE    _delay_us(9)
#define     OWI_DELAY_F_STD_MODE    _delay_us(55)
#define     OWI_DELAY_H_STD_MODE    _delay_us(480)
#define     OWI_DELAY_I_STD_MODE    _delay_us(70)
#define     OWI_DELAY_J_STD_MODE    _delay_us(410)

#define     OWI_ROM_READ    0x33
#define     OWI_ROM_SKIP    0xcc
#define     OWI_ROM_MATCH   0x55
#define     OWI_ROM_SEARCH  0xf0

#define OWI_PULL_BUS_LOW(bitMask) \
            DDR(OWI_PORT) |= bitMask; \
            OWI_PORT &= ~bitMask;

#define OWI_RELEASE_BUS(bitMask) \
            DDR(OWI_PORT) &= ~bitMask; \
            OWI_PORT |= bitMask;

void owi_init(uint8_t pins)
{
    OWI_RELEASE_BUS(pins);
    // The first rising edge can be interpreted by a slave as the end of a
    // Reset pulse. Delay for the required reset recovery time (H) to be
    // sure that the real reset is interpreted correctly.
    _delay_ms(1);
}

void owi_wr1(uint8_t pins)
{
    uint8_t g = SREG;
    cli();

    OWI_PULL_BUS_LOW(pins);
    OWI_DELAY_A_STD_MODE;
    OWI_RELEASE_BUS(pins);
    OWI_DELAY_B_STD_MODE;

    SREG = g;
}

void owi_wr0(uint8_t pins)
{
    uint8_t g = SREG;
    cli();

    OWI_PULL_BUS_LOW(pins);
    OWI_DELAY_C_STD_MODE;
    OWI_RELEASE_BUS(pins);
    OWI_DELAY_D_STD_MODE;

    SREG = g;
}

uint8_t owi_rdbit(uint8_t pins)
{
    uint8_t bitsRead;

    uint8_t g = SREG;
    cli();

    OWI_PULL_BUS_LOW(pins);
    OWI_DELAY_A_STD_MODE;
    OWI_RELEASE_BUS(pins);
    OWI_DELAY_E_STD_MODE;
    bitsRead = PIN(OWI_PORT) & pins;
    OWI_DELAY_F_STD_MODE;

    SREG = g;
    return bitsRead;
}

uint8_t owi_detect(uint8_t pins)
{
    uint8_t presenceDetected;

    uint8_t g = SREG;
    cli();

    OWI_PULL_BUS_LOW(pins);
    OWI_DELAY_H_STD_MODE;
    OWI_RELEASE_BUS(pins);
    OWI_DELAY_I_STD_MODE;
    presenceDetected = ((~PIN(OWI_PORT)) & pins);
    OWI_DELAY_J_STD_MODE;

    SREG = g;
    return presenceDetected;
}

// ----------------------------------------------------------------------------
// Higher functions
// ----------------------------------------------------------------------------

void owi_putb(uint8_t data, uint8_t pins)
{
    uint8_t temp;
    uint8_t i;

    // Do once for each bit
    for( i = 0; i < 8; i++ ) {
        // Determine if lsb is '0' or '1' and transmit corresponding waveform on the bus.
        temp = data & 0x01;
        if (temp) {
            owi_wr1(pins);
        } else {
            owi_wr0(pins);
        }
        // Right shift the data to get next bit.
        data >>= 1;
    }
}

uint8_t owi_getb(uint8_t pin)
{
    uint8_t data;
    uint8_t i;

    // Clear the temporary input variable.
    data = 0x00;

    // Do once for each bit
    for( i = 0; i < 8; i++ ) {
        // Shift temporary input variable right.
        data >>= 1;
        // Set the msb if a '1' value is read from the bus. Leave as it is ('0') else.
        if (owi_rdbit(pin)) {
            // Set msb
            data |= 0x80;
        }
    }
    return data;
}


void owi_skip(uint8_t pins)
{
    // Send the SKIP ROM command on the bus.
    owi_putb(OWI_ROM_SKIP, pins);
}

void owi_read(uint8_t* romValue, uint8_t pin)
{
    uint8_t bytesLeft = 8;

    // Send the READ ROM command on the bus.
    owi_putb(OWI_ROM_READ, pin);

    // Do 8 times.
    while( bytesLeft > 0 ) {
        // Place the received data in memory.
        *romValue++ = owi_getb(pin);
        bytesLeft--;
    }
}

void owi_match(uint8_t* romValue, uint8_t pins)
{
    uint8_t bytesLeft = 8;

    // Send the MATCH ROM command.
    owi_putb(OWI_ROM_MATCH, pins);

    // Do once for each byte.
    while( bytesLeft > 0 ) {
        // Transmit 1 byte of the ID to match.
        owi_putb(*romValue++, pins);
        bytesLeft--;
    }
}

// ----------------------------------------------------------------------------
// CRC functions
// ----------------------------------------------------------------------------

uint8_t owi_crc8(uint8_t inData, uint8_t seed)
{
    uint8_t bitsLeft;
    uint8_t temp;

    for( bitsLeft = 8; bitsLeft > 0; bitsLeft-- ) {
        temp = ((seed ^ inData) & 0x01);
        if( temp == 0 ) {
            seed >>= 1;
        } else {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
        }
        inData >>= 1;
    }
    return seed;
}

uint8_t owi_crcok(uint8_t* romValue)
{
    uint8_t i;
    uint8_t crc8 = 0;

    for( i = 0; i < 7; i++ ) {
        crc8 = owi_crc8(*romValue, crc8);
        romValue++;
    }

    return (crc8 == (*romValue));
}
