/*
 * mk_tools.c
 *
 *  Created on: 5 kwi 2022
 *      Author: admin
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>					// dla funkcji typu toupper() itp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "portmacro.h"
#include "FreeRTOSConfig.h"
#include "..\..\build\include\sdkconfig.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"


#include "esp_err.h"
#include "esp_log.h"

//-----------------------------------------







char * mk_upper( char * s ) {

	char * res = s;
	do {
		if( *s >= 'a' && *s <= 'z' ) *s &= ~0x20;
	} while( *s++ );
	return res;
}

char * mk_lower( char * s ) {

	char * res = s;
	do {
		if( *s >= 'A' && *s <= 'Z' ) *s |= 0x20;
	} while( *s++ );
	return res;
}
