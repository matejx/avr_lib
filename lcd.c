// ------------------------------------------------------------------
// --- lcd.c                                                      ---
// --- library for controlling the HD44780                        ---
// ---                                    coded by Matej Kogovsek ---
// ------------------------------------------------------------------

#include <inttypes.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "swdefs.h"

// ------------------------------------------------------------------
// --- procedures implemented by lcd_[intf].c -------------------------
// ------------------------------------------------------------------

extern uint8_t lcd_busw;
void lcd_hwinit(void);
void lcd_out(const uint8_t data, const uint8_t rs);
uint8_t lcd_command(const uint8_t cmd);
uint8_t lcd_data(const uint8_t data);

// ------------------------------------------------------------------
// --- private procedures -------------------------------------------
// ------------------------------------------------------------------

#ifdef LCD_USE_FB
static char lcd_fb[LCD_HEIGHT*(LCD_WIDTH+1)];
static uint8_t lcd_fbc;
#endif

static uint8_t lcd_cur;

// ------------------------------------------------------------------
// --- public procedures --------------------------------------------
// ------------------------------------------------------------------

// clear lcd
void lcd_clear(void)
{
	lcd_command(1);
	lcd_cur = 0;

#ifdef LCD_USE_FB
	memset(lcd_fb, ' ', sizeof(lcd_fb));
	lcd_fbc = lcd_cur;
#endif
}

// initialize lcd
void lcd_init(void)
{
	lcd_hwinit();
#ifndef LCD_SIMPLE_INIT
	lcd_out(0x30, 0);	// 8 bit interface
	_delay_ms(5);
	lcd_out(0x30, 0);
	_delay_ms(1);
	lcd_out(0x30, 0);
	_delay_ms(1);
	lcd_out(0x20 | lcd_busw, 0);
#endif
	lcd_command(0x28 | lcd_busw);	// 2 lines, 5x7 dots
	lcd_command(0x08);  // display off, cursor off, blink off
	lcd_clear();
	lcd_command(0x06);  // cursor increment
	lcd_command(0x08 | 0x04); // display on
}

// position lcd cursor to line y, character x (both zero based)
void lcd_goto(uint8_t x, uint8_t y)
{
#ifdef LCD_USE_FB
	lcd_fbc = (0x40 * y) + x;
#else
	lcd_cur = (0x40 * y) + x;
	lcd_command(0x80 + lcd_cur);
#endif

}

// position lcd cursor to the beginning of line l (one based)
void lcd_line(const uint8_t l)
{
	lcd_goto(0, l-1);
}

// write a char to lcd
void lcd_putc(const char c)
{
#ifdef LCD_USE_FB
	if( (lcd_fbc & 0x3f) >= LCD_WIDTH )
		return;

	uint8_t i = (lcd_fbc & 0x3f) + (lcd_fbc & 0x40 ? LCD_WIDTH+1 : 0);

	if( c != lcd_fb[i] ) {

		if( lcd_fbc != lcd_cur ) {
			lcd_command(0x80 + lcd_fbc);
			lcd_cur = lcd_fbc;
		}

		lcd_data(c);
		++lcd_cur;
		lcd_fb[i] = c;
	}
	++lcd_fbc;
#else
	if( (lcd_cur & 0x3f) >= LCD_WIDTH )
		return;

	lcd_data(c);
	++lcd_cur;
#endif
}

// write a string from progmem to lcd
#ifdef LCD_NEED_PUTSP
void lcd_puts_P(PGM_P s)
{
	uint8_t c;
	while ((c = pgm_read_byte(s++)))	{
		lcd_putc(c);
	}
}
#endif

// emulate endl by printing spaces until LCD width is reached
#ifdef LCD_NEED_ENDL
void lcd_endl(void)
{
	#ifdef LCD_USE_FB
	uint8_t* cur = &lcd_fbc;
	#else
	uint8_t* cur = &lcd_cur;
	#endif

	while( (0x3f & *cur) < LCD_WIDTH ) {
		lcd_putc(' ');
	}

	if( 0 == (0x40 & *cur) ) {
		lcd_goto(0, 1);
	}
}
#endif

// write a string from mem to lcd
#ifdef LCD_NEED_PUTS
uint8_t lcd_puts(const char* s)
{
  uint8_t len = 0;

	while( s[len] )	{
		lcd_putc(s[len]);
		len++;
	}

  return len;
}
#endif

// write a string of length n from mem to lcd
#ifdef LCD_NEED_PUTSN
void lcd_putsn(const char* s, uint8_t n)
{
	while(n--) {
		if (*s) {
			lcd_putc(*s);
		} else {
			lcd_putc(' ');
		}
		s++;
	}
}
#endif

// write an integer to lcd, prepending with leading zeros until minlen
#ifdef LCD_NEED_PUTI
void lcd_puti_lc(const uint32_t a, uint8_t r, uint8_t l, char c)
{
	char s[10];

	ltoa(a, s, r);

	while( l > strlen(s) ) {
		lcd_putc(c);
		l--;
	}

	lcd_puts(s);
}
#endif

// write a float to the lcd with prec decimals
#ifdef LCD_NEED_PUTF
void lcd_putf(float f, uint8_t prec)
{
	lcd_puti_lc(f, 10, 0, 0);
	lcd_putc('.');
	while( prec-- ) {
		f = f - (int)f;
		f = f * 10;
		lcd_puti_lc(f, 10, 0, 0);
	}
}
#endif
