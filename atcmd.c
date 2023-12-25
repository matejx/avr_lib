/**
@file		atc.c
@brief		AT command routines
@author		Matej Kogovsek
@copyright	LGPL 2.1
@note		This file is part of mat-avr-lib
*/

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "swdefs.h"
#include "serque.h"

// ------------------------------------------------------------------

char atc_buf[ATC_BUF_SIZE];

// ------------------------------------------------------------------

/**
@brief Wait a specified amount of msec for reply.
@param[in]	n				USART peripheral number
@param[in]	reply		Expected reply in PGMSPACE
@param[in]	to_ms		Timeout in msec
@return			0 on success, 1 on fail
*/
uint8_t atc_wait_reply(const uint8_t n, const PGM_P reply, const uint16_t to_ms)
{
	uint8_t d;
	uint8_t len = 0;
	uint16_t t_ms = 0;

	while( (t_ms <= to_ms) && (len < (sizeof(atc_buf)-1)) )
	{
		if( !ser_getc(n, &d) ) {
			atc_delay_ms(4);
			t_ms += 4;
			continue;
		}
		atc_buf[len++] = d;
		atc_buf[len] = 0;

		if( strstr_P(atc_buf, reply) ) {
			while( (t_ms <= to_ms) && (d != '\n') ) {
				if( !ser_getc(n, &d) ) {
					atc_delay_ms(2);
					t_ms += 2;
					continue;
				}
				if( len < (sizeof(atc_buf)-1) ) {
					atc_buf[len++] = d;
					atc_buf[len] = 0;
				}
			}

			return 0;	// reply found, return 0
		}
	}

	return 1;	// timeout, return 1
}

/**
@brief Send a command and wait for reply.
@param[in]	n				USART peripheral number
@param[in]	cmd1		Static part of command in PGMSPACE
@param[in]  cmd2		Possible variable part of command in RAM (or 0 if not used)
@param[in]	reply		Expected reply in PGMSPACE
@param[in]	to_ms		Timeout in msec
@return			0 on success, 1 on fail
*/
uint8_t atc_at_cmd(const uint8_t n, const PGM_P cmd1, const char* cmd2, const PGM_P reply, const uint16_t to_ms)
{
	ser_flush_rxbuf(n);
	if(cmd1) ser_puts_P(n, cmd1);
	if(cmd2) ser_puts(n, cmd2);
	ser_puts_P(n, PSTR("\r\n"));
	if( reply == 0 ) return 0;
	return atc_wait_reply(n, reply, to_ms);
}
