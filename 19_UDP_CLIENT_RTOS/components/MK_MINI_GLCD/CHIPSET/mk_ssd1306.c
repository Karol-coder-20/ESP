/*
 * mk_ssd1306.c
 *
 *  Created on: 25 mar 2022
 *      Author: Miros³aw Kardaœ
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

#include "mk_i2c.h"


#include "mk_glcd_config.h"
#include "mk_glcd_base.h"

#ifdef USE_SSD1306

#include "mk_ssd1306.h"





static void i2c_ssd1306_send( uint8_t ctrl, uint8_t dc ) {

    i2c_dev_write( 0, SSD1306_I2C_ADDR, &ctrl, 1, &dc, 1 );
}


static void mk_ssd1306_cmd( uint8_t cmd ) {

	i2c_ssd1306_send( 0x00, cmd );
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

	if( !can_display ) return;

	mk_ssd1306_cmd( 0x21  );			// Command_Column_Address_Set
	mk_ssd1306_cmd( 0x00  );			// Start
	mk_ssd1306_cmd( GLCD_WIDTH - 1  );	// End

	mk_ssd1306_cmd( 0x22  );			// Command_Page_Address_Set
	mk_ssd1306_cmd( 0x00  );			// Start
	mk_ssd1306_cmd( GLCD_HEIGHT - 1 );	// End

	mk_ssd1306_cmd( SSD1306_SETLOWCOLUMN  );
	mk_ssd1306_cmd( SSD1306_SETHIGHCOLUMN );

#if GLCD_HEIGHT == 64
	mk_ssd1306_cmd( 0xB0 );
#endif
#if GLCD_HEIGHT == 32
	mk_ssd1306_cmd( 0xB4 );
#endif

	uint8_t adr = 0x40;
	i2c_dev_write( 0, SSD1306_I2C_ADDR, &adr, 1, glcd_buf, GLCD_BUF_SIZE );

}





// contrast 0-255
void glcd_contrast( uint8_t contr ) {

	uint8_t buf[3];
	buf[0] = 0;
	buf[1] = SSD1306_SETCONTRAST;
	buf[2] = contr;
	i2c_dev_write( 0, SSD1306_I2C_ADDR, NULL, 0, buf, 3 );

}

//************* INICJALIZACJA sterownika SSD1306
void glcd_init( uint8_t contrast ) {


	ets_delay_us( 10000 );


    mk_ssd1306_cmd( SSD1306_DISPLAYOFF );
    mk_ssd1306_cmd( SSD1306_SETDISPLAYCLOCKDIV );
    mk_ssd1306_cmd( OLED_CONTRAST );

    mk_ssd1306_cmd( SSD1306_SETDISPLAYOFFSET );
    mk_ssd1306_cmd( 0x0);
    mk_ssd1306_cmd( SSD1306_SETSTARTLINE | 0x0 );
    mk_ssd1306_cmd( SSD1306_CHARGEPUMP );

//    if (vcc == SSD1306_EXTERNALVCC ) mk_ssd1306_cmd( 0x10 );
//    else  mk_ssd1306_cmd( 0x14 );	// SSD1306_SWITCHCAPVCC

    mk_ssd1306_cmd( 0x14 );	// SSD1306_SWITCHCAPVCC

    mk_ssd1306_cmd( SSD1306_MEMORYMODE );
    mk_ssd1306_cmd( 0x00);
    mk_ssd1306_cmd( SSD1306_SEGREMAP | 0x1 );
    mk_ssd1306_cmd( SSD1306_COMSCANDEC );


    mk_ssd1306_cmd( 0x00);
    mk_ssd1306_cmd( SSD1306_SETCONTRAST );
    mk_ssd1306_cmd( contrast );

//    if (vcc == SSD1306_EXTERNALVCC ) mk_ssd1306_cmd( 0x9F );
//    else mk_ssd1306_cmd( 0xCF );

    mk_ssd1306_cmd( 0xCF );	// SSD1306_SWITCHCAPVCC

    mk_ssd1306_cmd( SSD1306_SETPRECHARGE );

    // ssd1306 - 128 x 32
#ifdef GLCD_RES_128_32
	    mk_ssd1306_cmd( SSD1306_SETMULTIPLEX );
	    mk_ssd1306_cmd( 0x1F );

	    mk_ssd1306_cmd( SSD1306_SETCOMPINS );
	    mk_ssd1306_cmd( 0x02 );
#endif

	// ssd1306 - 128 x 64
#ifdef GLCD_RES_128_64
	    mk_ssd1306_cmd( SSD1306_SETMULTIPLEX );
	    mk_ssd1306_cmd( 0x3F );

	    mk_ssd1306_cmd( SSD1306_SETCOMPINS );
	    mk_ssd1306_cmd( 0x12 );
#endif

	    mk_ssd1306_cmd(SSD1306_SETVCOMDETECT);
	    mk_ssd1306_cmd(0x40);

		mk_ssd1306_cmd( SSD1306_DISPLAYALLON_RESUME );
		mk_ssd1306_cmd( SSD1306_NORMALDISPLAY );

	    mk_ssd1306_cmd( SSD1306_DISPLAYON );

#if SHOW_DEMO_SCREEN == 0
	    glcd_cls();
#endif
	    can_display = 1;
	    glcd_display();

	    setCurrentFont( &DefaultFont5x8 );
}




void glcd_inverse( uint8_t enable ) {
	if( enable ) mk_ssd1306_cmd( SSD1306_INVERTDISPLAY );
	else mk_ssd1306_cmd( SSD1306_NORMALDISPLAY );
}




void glcd_flip_vertical( uint8_t flip, uint8_t mirror ) {
	if( !flip ) {
		mk_ssd1306_cmd( 0x80 );		// normal
		mk_ssd1306_cmd( 0xC8 );
		mk_ssd1306_cmd( 0x80 );
		if( !mirror ) mk_ssd1306_cmd( 0xA1 );
		else mk_ssd1306_cmd( 0xA0 );
	} else {
		mk_ssd1306_cmd( 0x80 );		// flipped
		mk_ssd1306_cmd( 0xC0 );
		mk_ssd1306_cmd( 0x80 );
		if( !mirror ) mk_ssd1306_cmd( 0xA0 );
		else mk_ssd1306_cmd( 0xA1 );

	}
}










#endif








