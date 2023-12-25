/**

Interrupt driven. ADC takes ADC_AVG_SAMP (swdefs.h) and returns their average.

@file		adc.c
@brief		ADC routines
@author		Matej Kogovsek
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "swdefs.h"

static volatile uint16_t adc[8]; /**< Buffer of averaged results for all possible channels */
static uint8_t adc_ench; /**< Enabled channels */
static uint8_t adc_freerun; /**< Free running yes or no */
static volatile uint32_t adc_isum;
static volatile uint16_t adc_isamp;


#ifndef ADC_AVG_SAMP
#define ADC_AVG_SAMP 16		/**< How many ADC samples to average */
#endif

#if F_CPU >= 12000000 // half way between 16M and 8M
	#define ADC_PRESCALER _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) // 128
#elif F_CPU >= 6000000 // half way between 8M and 4M
	#define ADC_PRESCALER _BV(ADPS2) | _BV(ADPS1) // 64
#elif F_CPU >= 3000000 // half way between 4M and 2M
	#define ADC_PRESCALER _BV(ADPS2) | _BV(ADPS0) // 32
#elif F_CPU >= 1500000 // half way between 2M and 1M
	#define ADC_PRESCALER _BV(ADPS2) // 16
#else
	#define ADC_PRESCALER _BV(ADPS1) | _BV(ADPS0) // 8
#endif

/**
@brief Init ADC.
@param[in]	ench	Enabled channels (bitmask)
@param[in]	ref		ADC reference
*/
void adc_init(uint8_t ench, uint8_t ref)
{
	memset((void*)adc, 0xff, sizeof(adc)); // make all values invalid (0xffff)
	adc_isum = 0;
	adc_isamp = 0;
	adc_freerun = 0;
	adc_ench = ench;
	ADMUX = ref & (_BV(REFS1) | _BV(REFS0));
	// enable ADC and ADC int, prescaler
	ADCSRA = _BV(ADEN) | _BV(ADIE) | ADC_PRESCALER;
}

/**
@brief Disable ADC.
*/
void adc_shutdown(void)
{
	ADCSRA = 0;
}

/**
@brief Start next conversion.

Use if you want to control when conversions are started. Do not call if free running.
*/
void adc_startnext(void)
{
	if( ADCSRA & _BV(ADSC) ) return;

	if( adc_ench ) {
		uint8_t n = ADMUX & 7;

		do {
			n = (n + 1) & 7;
		} while( (adc_ench & _BV(n)) == 0 );

		ADMUX = (ADMUX & ~7) | n;

		ADCSRA |= _BV(ADSC);
	}
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
	adc_isum += ADC;					// add new conversion result to sum
	adc_isamp++;						// increase sample counter

	if( adc_isamp == ADC_AVG_SAMP ) {
		uint8_t n = ADMUX & 7;		    // get channel
		adc_isum /= ADC_AVG_SAMP;	    // calc average
		adc[n] = adc_isum;	            // store averaged conversion result
		#ifdef ADC_CALLBACK
		adc_callback(n, adc_isum);
		#endif
		adc_isamp = 0;				    // reset variables
		adc_isum = 0;
		if( adc_freerun ) {				// start next channel
			adc_startnext();
		}
	} else {
		ADCSRA |= _BV(ADSC);           // start next conversion
	}
}
