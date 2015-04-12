/**

Interrupt driven. ADC takes ADC_AVG_SAMP (swdefs.h) and returns their average.

@file		adc.c
@brief		ADC routines
@author		Matej Kogovsek (matej@hamradio.si)
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "swdefs.h"

static volatile uint16_t adc[8]; /**< Buffer of averaged results for all possible channels */
static uint8_t adc_nch; /**< Number of channels */
static uint8_t adc_freerun = 0; /**< Free running yes or no */

#ifndef ADC_AVG_SAMP
#define ADC_AVG_SAMP 16		/**< How many ADC samples to average */
#endif

/**
@brief Init ADC. Channels 0 to nch-1 will be sampled.
@param[in]	nch		Number of channels
*/
void adc_init(uint8_t nch)
{
	if( nch > 8 ) nch = 8;
	adc_nch = nch;
	ADMUX = _BV(REFS0);	// voltage reference = AVCC with cap on AREF
	// enable ADC and ADC int, prescaler = 64 (125kHz)
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1);
}

/**
@brief Start next conversion.

Use if you want to control when conversions are started. Do not call if free running.
*/
void adc_startnext(void)
{
	ADCSRA |= _BV(ADSC);
}

/**
@brief Start free running ADC conversions.

After a conversion is finished, a new conversion is automatically started.
*/
void adc_startfree(void)
{
	adc_freerun = 1;
	adc_startnext();
}

/**
@brief Get a channel's averaged ADC value.
@param[in]	ch		Channel to get
@return Averaged ADC value for channel
*/
uint16_t adc_get(const uint8_t ch)
{
	uint8_t g = SREG;
	cli();
	uint16_t r = adc[ch];
	SREG = g;
	return r;
}

/** @privatesection */

ISR(ADC_vect)
{
	static uint16_t sum = 0;
	static uint8_t samp = 0;

	sum += ADC;					    // add new conversion result to sum
	samp++;						    // increase sample counter

	if( samp == ADC_AVG_SAMP ) {
		uint8_t n = ADMUX & 7;	    // get channel
		sum /= ADC_AVG_SAMP;	    // calc average
		adc[n] = sum;	            // store averaged conversion result
		if( ++n >= adc_nch ) n = 0;	  // advance channel
		ADMUX = _BV(REFS0) | n;

		samp = 0;				    // reset variables
		sum = 0;
	}

	if( adc_freerun ) {
		ADCSRA |= _BV(ADSC);           // start next conversion
	}
}
