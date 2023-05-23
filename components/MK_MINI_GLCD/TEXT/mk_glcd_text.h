/*                  e-gadget.header
 * mk_glcd_text.h
 *
 *  Created on: 2019-05-21
 *    Modyfied: 2019-06-19 09:28:21
 *      Author: Miros³aw Kardaœ
 *         web: www.atnel.pl
 *
 *	Library: MK GLCD  ver: 1.1a
 *
 * 	 Obs³ugiwane wyœwietlacze/chipsety/magistrale:
 * 
 * 	 1. OLED 128x64 - SSD1306 I2C/SPI
 * 	 2. OLED 128x32 - SSD1306 I2C/SPI
 * 	 3. COG 128x64  - ST7565R SPI
 * 	 4. NOKIA 84x48 - PCD8544 SPI
 * 	 5. COG 128x64  - UC1701x SPI
 * 
 * 	 Biblioteka "MK GLCD" wymaga do prawid³owej pracy
 * 	 jednej z dwóch bibliotek:
 * 
 * 	 1. MK_I2C		https://sklep.atnel.pl/pl/p/0581_0582-MK-I2C-AVR-Biblioteka-C/244
 * 	 2. MK_SPI		https://sklep.atnel.pl/pl/p/0567_0570-MK-SPI-AVR-Biblioteka-C-/241
 * 
 * 	 F_CPU: 1MHz - 24MHz
 * 	 MCU: all ATmega uC with minimum 2Kb RAM
 * 	
 * 	 Uwaga! przed kompilacj¹ nale¿y dodaæ opcjê w ustawieniach toolchaina
 * 	 C/C++ Build/Settings/AVR C Linker/General/Other Arguments
 * 	 -Wl,-gc-sections
 * 	
 *
 *
 */

#include "../FONTS/mk_fonts.h"

#ifndef MK_GLCD_TEXT_MK_GLCD_TEXT_H_
#define MK_GLCD_TEXT_MK_GLCD_TEXT_H_

// u¿ywaj obs³ugi fontów w dwóch orientacjach T2B i R2L - wiêksze zu¿ycie Flash i czasu
#define USE_TWO_FONTS_BITORIENT			1		//0-disabled, 1-enabled


extern FONT_INFO currentFont;

void setCurrentFont( const FONT_INFO * font );

enum { ta_none, ta_left, ta_center, ta_right, ta_top, ta_bottom };

extern uint8_t glcd_h_align;
extern uint8_t glcd_v_align;


void glcd_locate( int x, int y );

void glcd_put_char( int x, int y, const char c, uint8_t mode );
void glcd_put_char1( const char c, uint8_t mode );

void glcd_puts( int x, int y, const char * s, uint8_t mode );
void glcd_puts1( const char * s, uint8_t mode );

void glcd_int( int x, int y, int32_t val, uint8_t mode );
void glcd_int1( int32_t val, uint8_t mode );

void glcd_uint( int x, int y, uint32_t val, uint8_t mode );
void glcd_uint1( uint32_t val, uint8_t mode );

void glcd_hex( int x, int y, uint32_t val, uint8_t mode );
void glcd_hex1( uint32_t val, uint8_t mode );

void glcd_bin( int x, int y, uint32_t val, uint8_t len, uint8_t mode );
void glcd_bin1( uint32_t val, uint8_t len, uint8_t mode );

int text_len( const char *s );
uint8_t char_len( const char c );


#endif /* MK_GLCD_TEXT_MK_GLCD_TEXT_H_ */















