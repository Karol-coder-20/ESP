/*
 * mk_glcd_base.h
 *
 *  Created on: 25 mar 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef COMPONENTS_MK_GLCD_MK_GLCD_BASE_H_
#define COMPONENTS_MK_GLCD_MK_GLCD_BASE_H_

#include "mk_glcd_common.h"
#include "mk_glcd_text.h"
#include "mk_glcd_config.h"
#include "mk_glcd_graphics.h"




typedef struct {
	uint8_t active;
	int left;
	int right;
	int top;
	int bottom;
} TVIEWPORT;

extern uint8_t glcd_buf[];

extern int cur_x;
extern int cur_y;

extern uint8_t szachownica;
extern int cur_x, cur_y;
extern uint8_t can_display;

extern TVIEWPORT viewport;





//-------------------------------------------------- FUNKCJE SPRZÊTOWE z CHIPSET xxx
// funkcje sprzêtowe - zwi¹zane z chipsetem
extern void glcd_init( uint8_t contrast );
extern void glcd_display( void );

// contrast 0-255
extern void glcd_contrast( uint8_t contr );
extern void glcd_flip_vertical( uint8_t flip, uint8_t mirror );
extern void glcd_inverse( uint8_t enable );

// funkcje sprzêtowe - zwi¹zane z tablic¹ pamiêci RAM Chipsetu
extern void glcd_set_pixel( int x, int y, uint8_t mode );
extern uint8_t glcd_get_pixel( int x, int y );
//-------------------------------------------------- KONIEC FUNKCJI SPRZÊTOWTCH z CHIPSET xxx


//.............. inne funkcje biblioteki
extern void glcd_cls( void );
extern void glcd_cls_viewport( void );




//------------- funkcje z GRAPHICS ------------------------------------------
void glcd_set_viewport( int x, int y, int width, int height );
void glcd_set_viewport_pages( uint8_t left, uint8_t right, uint8_t page, uint8_t count );
void glcd_set_viewport_x( uint8_t left, uint8_t right );
void glcd_reset_viewport( void );
void glcd_clear_viewport( uint8_t mode );


void glcd_drawBitmap( int x, int y, const uint8_t *bitmap,  uint8_t color );
void glcd_fast_drawBitmap( const uint8_t *bitmap );

void glcd_drawLine( int x0, int y0, int x1, int y1, uint8_t mode );
void glcd_fillRect( int x, int y, int w, int h, uint8_t mode );
void glcd_fillRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color);

void glcd_drawFastVLine( int x, int y, int h, uint8_t mode );
void glcd_drawFastHLine(int x, int y, int w, uint8_t mode);

void glcd_circle(int x, int y, uint8_t r, uint8_t bw);
void drawCircleQuads( int x0, int y0, int radius, uint8_t quads, uint8_t mode );
void glcd_fillCircle( int x, int y, uint8_t r, uint8_t mode );

void glcd_drawRect( int x, int y, int w, int h, uint8_t mode );
void glcd_drawRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t mode);

void glcd_drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t mode);
void glcd_fillTriangle ( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t mode);
//------------- funkcje z GRAPHICS KONIEC------------------------------------------





//------------- funkcje z COMMON ------------------------------------------
int32_t map32( int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max );
int map16( int x, int in_min, int in_max, int out_min, int out_max );
char * mkitoa( int32_t n, char * s, uint8_t len, char lead_sign );
char *strrev(char *str);
//------------- funkcje z COMMON KONIEC------------------------------------------


#endif /* COMPONENTS_MK_GLCD_MK_GLCD_BASE_H_ */
