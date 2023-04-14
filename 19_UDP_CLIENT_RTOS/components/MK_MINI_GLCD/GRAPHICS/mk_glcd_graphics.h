/*                  e-gadget.header
 * mk_glcd_graphics.h
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

#ifndef MK_GLCD_MK_GLCD_GRAPHICS_H_
#define MK_GLCD_MK_GLCD_GRAPHICS_H_

#define swap(a, b) { int16_t t = a; a = b; b = t; }















//void glcd_set_viewport( int x, int y, int width, int height );
//void glcd_set_viewport_pages( uint8_t left, uint8_t right, uint8_t page, uint8_t count );
//void glcd_set_viewport_x( uint8_t left, uint8_t right );
//void glcd_reset_viewport( void );
//void glcd_clear_viewport( uint8_t mode );
//
//
//void glcd_drawBitmap( int x, int y, const uint8_t *bitmap,  uint8_t color );
//void glcd_fast_drawBitmap( const uint8_t *bitmap );
//
//void glcd_drawLine( int x0, int y0, int x1, int y1, uint8_t mode );
//void glcd_fillRect( int x, int y, int w, int h, uint8_t mode );
//void glcd_fillRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color);
//
//void glcd_drawFastVLine( int x, int y, int h, uint8_t mode );
//void glcd_drawFastHLine(int x, int y, int w, uint8_t mode);
//
//void glcd_circle(int x, int y, uint8_t r, uint8_t bw);
//void drawCircleQuads( int x0, int y0, int radius, uint8_t quads, uint8_t mode );
//void glcd_fillCircle( int x, int y, uint8_t r, uint8_t mode );
//
//void glcd_drawRect( int x, int y, int w, int h, uint8_t mode );
//void glcd_drawRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t mode);
//
//void glcd_drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t mode);
//void glcd_fillTriangle ( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t mode);

#endif /* MK_GLCD_MK_GLCD_GRAPHICS_H_ */
















