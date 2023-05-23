/*                  e-gadget.header
 * mk_glcd_graphics.c
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


static void glcd_drawRleBitmap( int x, int y, const uint8_t * bitmap, uint8_t mode ) {


    uint8_t data=0, count=0;
    uint16_t i;
    uint8_t * p;
    int idx = x + ( (y>>3) * GLCD_WIDTH);		// Y - co osiem 0,8,16,...
    int x1 = x;
    p = glcd_buf;

    uint16_t rle_size = bitmap[1];
    int w  = bitmap[3];

    bitmap += 5;
    for (i = 5; i < rle_size; i++) {
    	do {
			if(count==0) {
				data = *bitmap++;
				if(data==*bitmap++) {
					count = *bitmap++;
					i+=2;
				}
				else {
					count = 1;
					bitmap--;
				}
			}
			if( x1 >= x+w ) {
					x1 = x;
					y += 8;
			}
			if( x1>=0 && x1<GLCD_WIDTH && y>=0 && y<GLCD_HEIGHT ) {
				idx = x1 + ( (y>>3) * GLCD_WIDTH);

				if( !mode ) p[ idx ] &= ~data;
				else if( 1==mode ) p[ idx ] |= data;
				else p[ idx ] ^= data;

			}
			x1++;
			count--;
    	} while( count );
    }
}


void glcd_fast_drawBitmap( const uint8_t *bitmap ) {
	if( bitmap[0] == 1 ) return;

	memcpy( glcd_buf, bitmap+5, GLCD_BUF_SIZE );
	can_display = 1;
}


void glcd_drawBitmap( int x, int y, const uint8_t *bitmap,  uint8_t mode ) {

	if( bitmap[0] == 1 ) {
		glcd_drawRleBitmap( x, y, bitmap, mode );
		can_display = 1;
		return;
	}

	uint8_t w = bitmap[3];
	uint8_t h = bitmap[1];
	bitmap += 5;

	if( !mode ) glcd_fillRect( x, y, w, h, 1 );
	else if( 1==mode ) glcd_fillRect( x, y, w, h, 0 );

	int i, j;

	// T2B
    for(i=0; i<h; i++ ) {
    	for(j=0; j<w; j++) {
    		if( *(bitmap + j + ((i/8)*w) ) & (1 << (i & 7)) ) {
	    	  glcd_set_pixel(x+j, y+i, mode);
    		} else {
    			if( 1==mode ) glcd_set_pixel( x+j, y+i, 0 );
    			else if( 0==mode ) glcd_set_pixel( x+j, y+i, 1 );
    			else if( 2==mode ) glcd_set_pixel( x+j, y+i, glcd_get_pixel(x+j, y+i) );
    			else glcd_set_pixel( x+j, y+i, !glcd_get_pixel(x+j, y+i) );
    		}
	    }
    }

//	  // L2R
//    uint8_t byteWidth = (w + 7) / 8;
//	  for(j=0; j<h; j++) {
//	    for(i=0; i<w; i++ ) {
//	      if( pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)) ) {
//	    	  glcd_set_pixel(x+i, y+j, mode);
//	      }
//	    }
//	  }
}


// Bresenham's algorithm - thx wikpedia
void glcd_drawLine( int x0, int y0, int x1, int y1, uint8_t mode ) {

	if( !szachownica ) {
	   int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
	   int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
	   int err = dx+dy, e2; /* error value e_xy */

	   for(;;){  /* loop */
		   glcd_set_pixel(x0,y0,mode);
	      if (x0==x1 && y0==y1) break;
	      e2 = 2*err;
	      if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
	      if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	   }
	} else {

		int steep = abs(y1 - y0) > abs(x1 - x0);
		if (steep) {
			swap(x0, y0);
			swap(x1, y1);
		}

		if (x0 > x1) {
			swap(x0, x1);
			swap(y0, y1);
		}

		int dx, dy;
		dx = x1 - x0;
		dy = abs(y1 - y0);

		int err = dx / 2;
		int ystep;

		if (y0 < y1) {
			ystep = 1;
		} else {
			ystep = -1;
		}

		for (; x0 <= x1; x0++) {
			// xxx
			if( 0 == szachownica ) {
				if (steep) {
					glcd_set_pixel(y0, x0, mode);
				} else {
					glcd_set_pixel(x0, y0, mode);
				}
			} else
			if( (szachownica == 0x01) ) {
				if( !(x0%2) ) {
					if (steep) {
						glcd_set_pixel(y0, x0, mode);
					} else {
						glcd_set_pixel(x0, y0, mode);
					}
				}
			} else
			if( (szachownica == 0x02) ) {
				if( (x0%2) ) {
					if (steep) {
						glcd_set_pixel(y0, x0, mode);
					} else {
						glcd_set_pixel(x0, y0, mode);
					}
				}
			}

			else if( (szachownica > 3) ) {
				uint8_t ax = (szachownica/2)-1;
				if( (x0%szachownica)<ax ) {
					if (steep) {
						glcd_set_pixel(y0, x0, mode);
					} else {
						glcd_set_pixel(x0, y0, mode);
					}
				}
			}

			err -= dy;
			if (err < 0) {
				y0 += ystep;
				err += dx;
			}
		}

	}

}



void glcd_fillRect( int x, int y, int w, int h, uint8_t mode ) {

	uint8_t am = szachownica;
	if( 3 == mode ) szachownica = 2;

	for( int ay = y; ay<y+h; ay++ ) {
		if( 3 == mode ) {
			szachownica ^= 2;
			szachownica ^= 1;
		}
		glcd_drawLine(x, ay, x+w-1, ay, mode);
	 }
	 szachownica = am;
}

void glcd_drawFastVLine( int x, int y, int h, uint8_t mode ) {
	glcd_drawLine(x, y, x, y+h-1, mode);

}

void glcd_drawFastHLine(int x, int y, int w, uint8_t mode) {
	glcd_drawLine(x, y, x+w-1, y, mode);

}


// rysowanie æwiartek okrêgu. Æwiartki przekazywane przez arg quads w postaci 4 m³odszych bitów.
// ka¿dy bit - jedna æwiartka
void drawCircleQuads( int x0, int y0, int radius, uint8_t quads, uint8_t mode ) {
	  int x = 0, y = radius;
	  int dp = 1 - radius;
	  while (x < y) {
		if (dp < 0)
		  dp = dp + 2 * (++x) + 3;
		else
		  dp = dp + 2 * (++x) - 2 * (--y) + 5;
		if (quads & 0x1) {
		  glcd_set_pixel( x0 + x, y0 - y, mode );
		  glcd_set_pixel( x0 + y, y0 - x, mode );
		}
		if (quads & 0x2) {
			glcd_set_pixel(x0 - y, y0 - x, mode );
			glcd_set_pixel(x0 - x, y0 - y, mode );
		}
		if (quads & 0x4) {
			glcd_set_pixel(x0 - y, y0 + x, mode );
			glcd_set_pixel(x0 - x, y0 + y, mode );
		}
		if (quads & 0x8) {
			glcd_set_pixel(x0 + x, y0 + y, mode );
			glcd_set_pixel(x0 + y, y0 + x, mode );
		}
	  }
	  if ( (quads & 0x1) && (quads & 0x8) ) {
		  glcd_set_pixel(x0 + radius, y0, mode );
	  }
	  if ( (quads & 0x4) && (quads & 0x8) ) {
		  glcd_set_pixel(x0, y0 + radius, mode );
	  }
	  if ( (quads & 0x2) && (quads & 0x4) ) {
		  glcd_set_pixel(x0 - radius, y0, mode );
	  }
	  if ( (quads & 0x1) && (quads & 0x2) ) {
		  glcd_set_pixel(x0, y0 - radius, mode );
	  }
}


// rysuj okr¹g
void glcd_circle( int x, int y, uint8_t r, uint8_t mode ) {
	int f = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int xa = 0;
	int ya = r;

	glcd_set_pixel(x, y + r, mode);
	glcd_set_pixel(x, y - r, mode);
	glcd_set_pixel(x + r, y, mode);
	glcd_set_pixel(x - r, y, mode);

	while (xa < ya) {
		if (f >= 0) {
			ya--;
			ddF_y += 2;
			f += ddF_y;
		}
		xa++;
		ddF_x += 2;
		f += ddF_x;

		glcd_set_pixel(x + xa, y + ya, mode);
		glcd_set_pixel(x - xa, y + ya, mode);
		glcd_set_pixel(x + xa, y - ya, mode);
		glcd_set_pixel(x - xa, y - ya, mode);
		glcd_set_pixel(x + ya, y + xa, mode);
		glcd_set_pixel(x - ya, y + xa, mode);
		glcd_set_pixel(x + ya, y - xa, mode);
		glcd_set_pixel(x - ya, y - xa, mode);
  }

}


// Used to do circles and roundrects
void glcd_fillCircleHelper( int x0, int y0, int r, uint8_t cornername, int delta, uint8_t mode ) {

	int f     = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x     = 0;
	int y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
    	glcd_drawFastVLine(x0+x, y0-y, 2*y+1+delta, mode);
    	glcd_drawFastVLine(x0+y, y0-x, 2*x+1+delta, mode);
    }
    if (cornername & 0x2) {
    	glcd_drawFastVLine(x0-x, y0-y, 2*y+1+delta, mode);
    	glcd_drawFastVLine(x0-y, y0-x, 2*x+1+delta, mode);
    }
  }

}


void glcd_fillCircle( int x, int y, uint8_t r, uint8_t mode ) {
	glcd_drawFastVLine( x, y-r, 2*r+1, mode);
	glcd_fillCircleHelper( x, y, r, 3, 0, mode);

}


void glcd_drawRect( int x, int y, int w, int h, uint8_t mode ) {
	glcd_drawFastHLine(x, y, w, mode);
	glcd_drawFastHLine(x, y+h-1, w, mode);
	glcd_drawFastVLine(x, y, h, mode);
	glcd_drawFastVLine(x+w-1, y, h, mode);

}

void glcd_drawCircleHelper( int x0, int y0, int8_t r, uint8_t cornername, uint8_t mode) {
	int f     = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x     = 0;
	int y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
    	glcd_set_pixel(x0 + x, y0 + y, mode);
    	glcd_set_pixel(x0 + y, y0 + x, mode);
    }
    if (cornername & 0x2) {
    	glcd_set_pixel(x0 + x, y0 - y, mode);
    	glcd_set_pixel(x0 + y, y0 - x, mode);
    }
    if (cornername & 0x8) {
    	glcd_set_pixel(x0 - y, y0 + x, mode);
    	glcd_set_pixel(x0 - x, y0 + y, mode);
    }
    if (cornername & 0x1) {
    	glcd_set_pixel(x0 - y, y0 - x, mode);
    	glcd_set_pixel(x0 - x, y0 - y, mode);
    }
  }
}

void glcd_drawRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t mode) {

	glcd_drawFastHLine(x+r  , y    , w-2*r, mode);
	glcd_drawFastHLine(x+r  , y+h-1, w-2*r, mode);
	glcd_drawFastVLine(x    , y+r  , h-2*r, mode);
	glcd_drawFastVLine(x+w-1, y+r  , h-2*r, mode);

	glcd_drawCircleHelper(x+r    , y+r    , r, 1, mode);
	glcd_drawCircleHelper(x+w-r-1, y+r    , r, 2, mode);
	glcd_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, mode);
	glcd_drawCircleHelper(x+r    , y+h-r-1, r, 8, mode);

}



void glcd_drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t mode) {
	glcd_drawLine(x0, y0, x1, y1, mode);
	glcd_drawLine(x1, y1, x2, y2, mode);
	glcd_drawLine(x2, y2, x0, y0, mode);

}


void glcd_fillTriangle ( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t mode) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    glcd_drawFastHLine(a, y0, b-a+1, mode);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    glcd_drawFastHLine(a, y, b-a+1, mode);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    glcd_drawFastHLine(a, y, b-a+1, mode);
  }

}


void glcd_fillRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color) {
	glcd_fillRect(x+r, y, w-2*r, h, color);

	glcd_fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
	glcd_fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}



void glcd_set_viewport( int x, int y, int width, int height ) {
	viewport.left = x;
	viewport.right = x+width-1;
	viewport.top = y;
	viewport.bottom = y+height-1;
	viewport.active = 1;
}

void glcd_set_viewport_pages( uint8_t left, uint8_t right, uint8_t page, uint8_t count ) {
	viewport.left = left;
	viewport.right = right;
	viewport.top = page * 8;
	viewport.bottom = (count * 8) - 1;
	viewport.active = 1;
}

void glcd_set_viewport_x( uint8_t left, uint8_t right ) {
	viewport.left = left;
	viewport.right = right;
	viewport.top = 0;
	viewport.bottom = GLCD_HEIGHT;
	viewport.active = 1;
}

void glcd_reset_viewport( void ) {
	viewport.active = 0;
	viewport.left = 0;
	viewport.right = GLCD_WIDTH;
	viewport.top = 0;
	viewport.bottom = GLCD_HEIGHT;
}

void glcd_clear_viewport( uint8_t mode ) {
	glcd_fillRect( viewport.left, viewport.top, viewport.right-viewport.left+1, viewport.bottom-viewport.top+1, mode );
}




















