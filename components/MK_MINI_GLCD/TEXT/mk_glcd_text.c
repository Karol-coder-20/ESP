/*                  e-gadget.header
 * mk_glcd_text.c
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_task_wdt.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/semphr.h"


#include "portmacro.h"
#include "FreeRTOSConfig.h"
#include "..\..\build\include\sdkconfig.h"

//------------------------------------------------

#include "../mk_glcd_base.h"
//#include  "mk_glcd_text.h"
//
//#include "../FONTS/mk_fonts.h"
//#include "../mk_glcd_config.h"

uint8_t glcd_h_align;
uint8_t glcd_v_align;

FONT_INFO currentFont;

void setCurrentFont( const FONT_INFO * font ) {
	currentFont.down_space			= font->down_space;
	currentFont.heightPixels 		= font->heightPixels;
	currentFont.startChar 			= font->startChar;
	currentFont.interspacePixels	= font->interspacePixels;
	currentFont.spacePixels 		= font->spacePixels;
	currentFont.charInfo 			= (FONT_CHAR_INFO*)font->charInfo;
	currentFont.data 				= (uint8_t*)font->data;
	currentFont.FontFileName		= (char*)font->FontFileName;
#if USE_TWO_FONTS_BITORIENT == 1
	currentFont.bitOrientation		= font->bitOrientation;
#endif
}








static void send_glyph_byte( int x, int y, uint8_t bajt, uint8_t height, uint8_t mode ) {

	for( uint8_t i=0; i<height; i++ ) {
		if( bajt & 0x01 ) glcd_set_pixel( x,y, mode );
//		else {
//			set_background_pixel( x, y, mode );
//		}
		bajt>>=1;
		y++;
	}
}


#if USE_TWO_FONTS_BITORIENT == 1
static void send_glyph_byteB( int x, int y, uint8_t bajt, uint8_t width, uint8_t mode ) {

	for( uint8_t i=0; i<width; i++ ) {
		if( bajt & 0x80 ) glcd_set_pixel( x,y, mode );
//		else {
//			set_background_pixel( x, y, mode );
//		}
		bajt<<=1;
		x++;
	}
}


static void send_font_bitmapB( int x, int y, const uint8_t *glyph, int glyphHeight, int glyphWidth, uint8_t mode ) {

	uint8_t i, j, k, bajt;
	uint16_t p=0;


	for( i=0; i<glyphHeight; i++ ) {
		for( j=0, k=0; j<glyphWidth; j+=8, k++ ) {
			bajt = glyph[ p++ ];
			if( ((k+1)*8)<=glyphWidth ) send_glyph_byteB( x+(k*8), y, bajt, 8, mode );
			else send_glyph_byteB( x+(k*8), y, bajt, glyphWidth-(k*8), mode );
		}
		y++;
	}

}
#endif


static void send_font_bitmap( int x, int y, const uint8_t *glyph, int glyphHeight, int glyphWidth, uint8_t mode ) {

	uint8_t i, j, k, bajt;
	uint16_t p=0;

	for( i=0; i<glyphWidth; i++ ) {
		for( j=0, k=0; j<glyphHeight; j+=8, k++ ) {
			bajt = glyph[ p++ ];
			if( ((k+1)*8)<=glyphHeight ) send_glyph_byte( x, y+(k*8), bajt, 8, mode );
			else send_glyph_byte( x, y+(k*8), bajt, glyphHeight-(k*8), mode );
		}
		x++;
	}

}



void glcd_put_char( int x, int y, const char c, uint8_t mode ) {

	uint8_t gH, gW, gS, gIS;//,i;
	uint16_t offset;
	uint8_t startChar = currentFont.startChar;
	uint8_t * glyph = (uint8_t*)currentFont.data;

	gH = currentFont.heightPixels;
	gIS = currentFont.interspacePixels;
	gS = currentFont.spacePixels;


#if USE_TWO_FONTS_BITORIENT == 1
	void (*fun)( int x, int y, const uint8_t *glyph, int glyphHeight, int glyphWidth, uint8_t mode );

	if( currentFont.bitOrientation == 0 ) fun = send_font_bitmap;
	else fun = send_font_bitmapB;

#endif


	if( c && c != ' ' ) {

		gW = currentFont.charInfo[ c - startChar  ].widthBits;
		offset = currentFont.charInfo[ c - startChar  ].offset;
		if( (x + gW - 1 ) >= 0 && x<GLCD_WIDTH ) {
#if USE_TWO_FONTS_BITORIENT == 1
			fun(x, y, glyph+offset, gH, gW, mode );
#else
			send_font_bitmap(x, y, glyph+offset, gH, gW, mode );
#endif
		}
		x = x + gW + gIS;
	} else {
//		// rysowanie spacji
//		for(offset=0;offset<gS;offset++) {
//			for(uint8_t i=0;i<gH;i++) {
//				glcd_set_pixel( x+offset, y+i, 0 );
//			}
//		}
		x += gS + gIS;
	}
	cur_x = x;
	cur_y = y;
}


void glcd_put_char1( const char c, uint8_t mode ) {
	glcd_put_char( cur_x, cur_y, c, mode );
}


void glcd_puts( int x, int y, const char * s, uint8_t mode ) {

	if( !viewport.active ) {
		if( ta_center == glcd_h_align ) x = GLCD_WIDTH/2 - text_len(s)/2 + currentFont.interspacePixels/2;
		else if( ta_right == glcd_h_align ) x = GLCD_WIDTH - text_len(s) + currentFont.interspacePixels + (strlen(s)%2);
		else if( ta_left == glcd_h_align ) x = 0;

		if( ta_center == glcd_v_align ) y = GLCD_HEIGHT/2 - currentFont.heightPixels/2;
		else if( ta_top == glcd_v_align ) y = 0;
		else if( ta_bottom == glcd_v_align ) y = GLCD_HEIGHT - currentFont.heightPixels;
	} else {
		if( ta_center == glcd_h_align ) {
			int val = ((viewport.right-viewport.left)/2)+viewport.left;
			x = val - text_len(s)/2 + currentFont.interspacePixels/2;
		}
		else if( ta_right == glcd_h_align ) {
			int val = viewport.right;
			x = val - text_len(s) + currentFont.interspacePixels + (strlen(s)%2);
		}
		else if( ta_left == glcd_h_align ) {
			x = viewport.left;
		}

		if( ta_center == glcd_v_align ) {
			int val = ((viewport.bottom-viewport.top)/2)+viewport.top;
			y = val - currentFont.heightPixels/2;
		}
		else if( ta_top == glcd_v_align ) y = viewport.top;
		else if( ta_bottom == glcd_v_align ) y = viewport.bottom - currentFont.heightPixels+1;
	}

	cur_x = x;
	cur_y = y;

	if( mode<2 ) glcd_fillRect(x,y, text_len(s), currentFont.heightPixels, mode^1 );

	while( *s ) glcd_put_char( cur_x, cur_y, *s++, mode );
}

void glcd_puts1( const char * s, uint8_t mode ) {

	glcd_puts( cur_x, cur_y, s, mode );
}


void glcd_int1( int32_t val, uint8_t mode ) {
	glcd_int( cur_x, cur_y, val, mode );
}

void glcd_uint( int x, int y, uint32_t val, uint8_t mode ) {
	char buf[17];
//	ultoa(val, buf, 10 );
	sprintf( buf,"%d", val );

	glcd_puts( x, y, buf, mode );
}

void glcd_uint1( uint32_t val, uint8_t mode ) {
	glcd_uint( cur_x, cur_y, val, mode );
}


void glcd_hex( int x, int y, uint32_t val, uint8_t mode ) {
	char buf[5];
//	ultoa(val, buf, 16 );
	sprintf( buf,"%d", val );

	glcd_puts( x, y, buf, mode );
}

void glcd_hex1( uint32_t val, uint8_t mode ) {
	glcd_hex( cur_x, cur_y, val, mode );
}


// konwersja do postaci binarnej liczb max 32-bitowych
// ARG:
// val - liczba do konwersji
// len - iloœæ znaków postaci binarnej z zerami nieznacz¹cymi
void glcd_bin( int x, int y, uint32_t val, uint8_t len, uint8_t mode ) {
	char str[len+1];
	memset( str, 0, len+1 );
	for( int8_t i=0, k=len-1; i<len; i++ ) {
		uint32_t a = val >> k;
		if( a & 0x0001 ) str[k]='1'; else str[k]='0';
		k--;
	}

	strrev( str );

	glcd_puts( x, y, str, mode );
}

void glcd_bin1( uint32_t val, uint8_t len, uint8_t mode ) {
	glcd_bin( cur_x, cur_y, val, len, mode );
}


void glcd_locate( int x, int y ) {
	cur_x = x;
	cur_y = y;
}


uint8_t char_len( const char c ) {
	int x=0;
	uint8_t gW, gS, gIS;
	uint8_t startChar = currentFont.startChar;

	gIS = currentFont.interspacePixels;
	gS = currentFont.spacePixels;

	if( c > ' ') {
		gW = currentFont.charInfo[ c - startChar  ].widthBits;
		x = x + gW + gIS;
	} else x=gS+gIS;


	return x;
}

int text_len( const char *s ) {

	int x=0;
	uint8_t gW, gS, gIS;
	uint8_t startChar = currentFont.startChar;

	gIS = currentFont.interspacePixels;
	gS = currentFont.spacePixels;

	while( *s ) {
		if( *s > ' ') {
			gW = currentFont.charInfo[ *s - startChar  ].widthBits;
			x = x + gW + gIS;
		} else x+=gS+gIS;
		s++;
	}

	return x;
}


















