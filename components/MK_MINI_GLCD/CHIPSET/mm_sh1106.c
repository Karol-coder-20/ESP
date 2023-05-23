/*
 * mm_sh1106.c
 *
 *  Created on: 04 Apr 2022
 *      Author: rogerpolaris
 */

/* =========================================================== */

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
/* =========================================================== */


#include "mk_i2c.h"

#include "mk_glcd_config.h"
#include "mk_glcd_base.h"

#ifdef USE_SH1106

#include "mm_sh1106.h"



static void mm_sh1106_cmd( uint8_t Command){

	uint8_t a = 0x00;

	i2c_dev_write( 0, SH1106_I2C_ADDR, &a, 1, &Command, 1);
}

static void mm_sh1106_data( uint8_t *Data, uint16_t Size){

	uint8_t a = 0x40;

	i2c_dev_write( 0, SH1106_I2C_ADDR, &a, 1, Data, Size);
}

void glcd_set_pixel( int x, int y, uint8_t mode ) {

	if( (x < 0) || (x > GLCD_WIDTH-1) || (y < 0) || (y > GLCD_HEIGHT-1 ) ) return;

	if( viewport.active ) {
		if ((x < viewport.left) || (x > viewport.right) || (y < viewport.top) || (y > viewport.bottom ))
			return;
	}

	//------------ bytes - vertical T2B -----------------------------------------------------
	int idx = x + (y/8)*GLCD_WIDTH;
	uint8_t bit = (1<<(y%8));

	if( !mode ) glcd_buf[idx] &= ~(bit);			// 0 - zgaœ pixel
	else if( 1 == mode ) glcd_buf[idx] |= (bit); 	// 1 - zapal pixel
	else glcd_buf[idx] ^= (bit);					// 2 - zmieñ na przeciwny (XOR)

	can_display = 1;
}

uint8_t glcd_get_pixel( int x, int y ) {

	int idx = x + (y/8)*GLCD_WIDTH;
	uint8_t bit = (1<<(y%8));

	uint8_t res = ( glcd_buf[ idx ] & bit ) > 0;

	return res;
}

void glcd_display( void ) {

	uint8_t m_col = 2;

	for ( uint8_t i = 0; i < GLCD_HEIGHT/8; i++){

		mm_sh1106_cmd(0xB0 + i);     									//set page address
	    mm_sh1106_cmd(SH1106_SETLOWCOLUMN | (m_col & 0x0F));          	//set lower column address
	    mm_sh1106_cmd(SH1106_SETHIGHCOLUMN | ((m_col & 0xF0)  >> 4));  	//set higher column address

	    mm_sh1106_data(&glcd_buf[GLCD_WIDTH * i], GLCD_WIDTH);
	}
}

void glcd_contrast( uint8_t contr ) {

	uint8_t buf[3];
	buf[0] = 0;
	buf[1] = SH1106_SETCONTRAST;
	buf[2] = contr;
	i2c_dev_write( 0, SH1106_I2C_ADDR, NULL, 0, buf, 3 );
}

void glcd_inverse( uint8_t enable ){

	if( enable ) mm_sh1106_cmd( SH1106_INVERTDISPLAY );
		else mm_sh1106_cmd( SH1106_NORMALDISPLAY );
}

void glcd_flip_vertical( uint8_t flip, uint8_t mirror ) {

	if( !flip ) {
		mm_sh1106_cmd( 0x80 );		// normal
		mm_sh1106_cmd( SH1106_COMSCANDEC );
		mm_sh1106_cmd( 0x80 );
		if( !mirror ) mm_sh1106_cmd( SH1106_SETSEGMENTREMAP_RD );
		else mm_sh1106_cmd( SH1106_SETSEGMENTREMAP_ND );
	} else {
		mm_sh1106_cmd( 0x80 );		// flipped
		mm_sh1106_cmd( SH1106_COMSCANINC );
		mm_sh1106_cmd( 0x80 );
		if( !mirror ) mm_sh1106_cmd( SH1106_SETSEGMENTREMAP_ND );
		else mm_sh1106_cmd( SH1106_SETSEGMENTREMAP_RD );
	}
}

void glcd_init( uint8_t contrast ) {

	ets_delay_us( 10000 );

	mm_sh1106_cmd( SH1106_DISPLAYOFF );

	mm_sh1106_cmd( SH1106_NORMALDISPLAY );

	mm_sh1106_cmd( SH1106_SETDISPLAYCLOCKDIV );
	mm_sh1106_cmd( 0x80);

	mm_sh1106_cmd( SH1106_SETMULTIPLEX );
	mm_sh1106_cmd( 0x3F );

	mm_sh1106_cmd( SH1106_SETCOMPINS );
	mm_sh1106_cmd( 0x12 );

	mm_sh1106_cmd( SH1106_SETDISPLAYOFFSET );
	mm_sh1106_cmd( 0x00 );

	mm_sh1106_cmd(SH1106_SETSTARTLINE | 0x00);

	mm_sh1106_cmd(SH1106_CHARGEPUMP);
	mm_sh1106_cmd( 0x14 );

	mm_sh1106_cmd( SH1106_MEMORYMODE );
	mm_sh1106_cmd( 0x00);

	mm_sh1106_cmd( SH1106_SETSEGMENTREMAP_RD );

	mm_sh1106_cmd( SH1106_COMSCANDEC );

	mm_sh1106_cmd(SH1106_SETCONTRAST);
	mm_sh1106_cmd( contrast );

	mm_sh1106_cmd(SH1106_SETPRECHARGE);
	mm_sh1106_cmd( 0x22 );

	mm_sh1106_cmd( SH1106_SETVCOMDETECT);
	mm_sh1106_cmd( 0x40 );

	mm_sh1106_cmd(SH1106_DISPLAYALLON_RESUME);

	mm_sh1106_cmd( SH1106_NORMALDISPLAY );
	mm_sh1106_cmd( 0x2E );

	mm_sh1106_cmd( SH1106_DISPLAYON );

	mm_sh1106_cmd( SH1106_SETLOWCOLUMN | 0x0 );
	mm_sh1106_cmd( SH1106_SETHIGHCOLUMN | 0x0 );
	mm_sh1106_cmd( SH1106_SETSTARTLINE | 0x0 );

#if SHOW_DEMO_SCREEN == 0
	    glcd_cls();
#endif

	can_display = 1;
	glcd_display();

	setCurrentFont( &DefaultFont5x8 );
}


#endif
