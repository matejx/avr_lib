// ------------------------------------------------------------------
// --- adc.c                                                      ---
// --- library for reading the AVR ADC                            ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint16_t adc[8];
static uint8_t adc_nch;
static uint8_t adc_freerun = 0;

#define ADC_AVG_SAMP 16		// how many ADC samples to average

// sets up ADC conversion complete interrupt and starts converting
void adc_init(uint8_t nch) 
{
	if( nch > 8 ) nch = 8;
	adc_nch = nch;
	ADMUX = _BV(REFS0);	// voltage reference = AVCC with cap on AREF
	// enable ADC and ADC int, prescaler = 64 (125kHz)
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1); 
}

void adc_startnext(void)
{
	ADCSRA |= _BV(ADSC);
}

void adc_startfree(void)
{
	adc_freerun = 1;
	adc_startnext();
}

// get the latest adc[i] value
uint16_t adc_get(const uint8_t i)
{
	uint8_t g = SREG;
	cli();
	uint16_t r = adc[i];
	SREG = g;
	return r;
}

// ADC conversion complete interrupt
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
