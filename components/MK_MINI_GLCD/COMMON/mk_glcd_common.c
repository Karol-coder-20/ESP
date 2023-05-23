/*                  e-gadget.header
 * mk_glcd_common.c
 *
 *  Created on: 2019-06-15
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




char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}



int32_t map32( int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max ) {

	return ((x - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;
}


int map16( int x, int in_min, int in_max, int out_min, int out_max ) {

	return ( (x - in_min) * (out_max - out_min)) / (in_max - in_min) + out_min;
}



char * mkitoa( int32_t n, char * s, uint8_t len, char lead_sign ) {
    int32_t i, sign;
    len--;

    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10l + '0';
        if(i>len) break;
    } while ((n /= 10l) > 0);

    if( lead_sign == 0 ) {
		if (sign < 0) s[i++] = '-';
		s[i] = '\0';
    } else {


		if( lead_sign == ' ' ) {
			if (sign < 0) s[i++] = '-';
			if (sign < 0) len++;
		}

		uint8_t dl = strlen(s);
		len++;
		if (sign < 0) len--;
		int8_t dif = len - dl;

		if( dif > 0 ) {
			for( uint8_t j=0; j<dif; j++, i++ ) s[dl+j] = lead_sign;
		}

		if( lead_sign != ' ' ) {
			if (sign < 0) s[i++] = '-';
			s[i] = '\0';
		}
    }

    return strrev(s);
}








