/*	WIFI
 *  UDP Client
 *  dla ESP8266 RTOS
 *
 * 	main.c
 *
 * 	autor: Miros�aw Karda�
 * 	web: 	www.akademia.atnel.pl
 * 			www.atnel.pl
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
#include "..\build\include\sdkconfig.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"


#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <errno.h>


#include "esp_err.h"
#include "esp_log.h"

//-----------------------------------------

#include "mk_i2c.h"
#include "mk_glcd_base.h"
//#include "ds18x20.h"

#include "mk_wifi.h"
#include "mk_tools.h"




#define RED		"\x1b""[0;31m"		// Red
#define LIM		"\x1b""[0;32m"		// Lime
#define YEL		"\x1b""[0;33m"		// Yellow
#define BLU		"\x1b""[0;34m"		// Blue
#define MAG		"\x1b""[0;35m"		// Magenta (Fuchsia)
#define AQU		"\x1b""[0;36m"		// Aqua

#define GRE		"\x1b""[0;37m"		// Green
#define BRE		"\x1b""[0;38m"		// Bright Red
#define BBL		"\x1b""[0;39m"		// Bright Blue

#define TCLS	"\x1b""[2J"			// CLS Terminal


//#define LED1_GPIO 		2

/*************************************************************************************************************************/
#define SIMPLE_CLIENT_EXAMPLE		1			// 1-najprostszy przyk�ad klienta UDP, 0-zaawansowany przyk�ad klienta UDP
/*************************************************************************************************************************/

/*:::: praca ::::*/
#ifdef PRACA
#define UDP_REMOTE_ADDR		"192.168.1.188"
#define UDP_REMOTE_ADDR2	"192.168.1.100"
#define UDP_REMOTE_PORT		8888
#endif


/*:::: dom ::::*/
#ifdef DOM
#define UDP_REMOTE_ADDR		"192.168.2.130"
#define UDP_REMOTE_ADDR2	"192.168.2.116"
#define UDP_REMOTE_PORT		8888
#endif

/*:::: telefon ::::*/
#ifdef TELEFON
#define UDP_REMOTE_ADDR		"192.168.89.127"
#define UDP_REMOTE_ADDR2	"192.168.89.128"
#define UDP_REMOTE_PORT		8888
#endif



#if SIMPLE_CLIENT_EXAMPLE == 0
QueueHandle_t xQueueClientUDP;

typedef struct {
	char data[100];
} TQUEUE_UDP_CLI_DATA;

#endif

static const char *TAG = "UDP CLIENT: ";





#if SIMPLE_CLIENT_EXAMPLE == 1

TaskHandle_t udp_client_task_handle;

static void udp_client_task( void * arg ) {

	uint8_t licznik = 0;
	eTaskGetState(udp_client_task_handle);

	while(1) {

		struct sockaddr_in destAddr;
		destAddr.sin_addr.s_addr = inet_addr( UDP_REMOTE_ADDR );
		destAddr.sin_family = AF_INET;
		destAddr.sin_port = htons( UDP_REMOTE_PORT );

		int udp_cli_sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
		if (udp_cli_sock < 0) {
			ESP_LOGE( TAG, "Unable to create socket: errno %d", errno );
			break;
		}
		ESP_LOGI(TAG, "UDP CLIENT Socket created");

		while(1) {

			char data_to_send[50];
			sprintf( data_to_send, "Licznik: %d\n", licznik++ );
			int err = sendto(udp_cli_sock, data_to_send, strlen(data_to_send), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
			if (err < 0) {
				ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
				break;
			}
			vTaskDelay( 100 );
		}

        if (udp_cli_sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(udp_cli_sock, 0);
            close(udp_cli_sock);
            break;		// je�li NIE chcemy kasowa� w�tku w callbacku OnDisconnect
        }

        vTaskDelay( 100 );
	}

	ESP_LOGW(TAG, "UDP CLIENT TASK - DELETE...");
	vTaskDelay( 100 );
	vTaskDelete(NULL);
}
#endif






#if SIMPLE_CLIENT_EXAMPLE == 0
static void udp_client_task( void * arg ) {

	BaseType_t xStatus;
	TQUEUE_UDP_CLI_DATA dane;

	while(1) {

		struct sockaddr_in destAddr;
		destAddr.sin_addr.s_addr = inet_addr( UDP_REMOTE_ADDR );
		destAddr.sin_family = AF_INET;
		destAddr.sin_port = htons( UDP_REMOTE_PORT );

		int udp_cli_sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
		if (udp_cli_sock < 0) {
			ESP_LOGE( TAG, "Unable to create socket: errno %d", errno );
			break;
		}
		ESP_LOGI(TAG, "UDP CLIENT Socket created");

		while(1) {

			memset( dane.data, 0, sizeof(dane.data) );
			xStatus = xQueueReceive( xQueueClientUDP, &dane, portMAX_DELAY );	// dane z kolejki

			if( xStatus == pdPASS ) {

				int err = sendto(udp_cli_sock, dane.data, strlen(dane.data), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
				if (err < 0) {
					ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
				}
			}
		}

        if (udp_cli_sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(udp_cli_sock, 0);
            close(udp_cli_sock);
            break;
        }
	}

	ESP_LOGW(TAG, "UDP CLIENT TASK - DELETE...");
	vTaskDelay( 100 );
	vTaskDelete(NULL);
}
#endif










void mk_got_ip_cb( char * ip ) {

	szachownica = 0;
	glcd_drawRect( 0, 6, 128, 29, 1 );
	setCurrentFont( &DefaultFont5x8 );
	glcd_puts( 7, 10, "STA connected, IP:", 1 );
	setCurrentFont( &font_bitocra5x13 );
	glcd_puts( 7, 20, ip, 1 );
	glcd_display();

	tcpip_adapter_ip_info_t ip_info;

	tcpip_adapter_get_ip_info( TCPIP_ADAPTER_IF_STA, &ip_info );
	printf( "[STA] IP: " IPSTR "\n", IP2STR(&ip_info.ip) );
	printf( "[STA] MASK: " IPSTR "\n", IP2STR(&ip_info.netmask) );
	printf( "[STA] GW: " IPSTR "\n", IP2STR(&ip_info.gw) );

	tcpip_adapter_get_ip_info( TCPIP_ADAPTER_IF_AP, &ip_info );
	printf( "[AP] IP: " IPSTR "\n", IP2STR(&ip_info.ip) );
	printf( "[AP] MASK: " IPSTR "\n", IP2STR(&ip_info.netmask) );
	printf( "[AP] GW: " IPSTR "\n", IP2STR(&ip_info.gw) );

#if SIMPLE_CLIENT_EXAMPLE == 0
	xTaskCreate( udp_client_task, "", 4096, NULL, 1, NULL  );
#endif

#if SIMPLE_CLIENT_EXAMPLE == 1
	xTaskCreate( udp_client_task, "", 4096, NULL, 1, &udp_client_task_handle  );
#endif
}


void mk_sta_disconnected_cb( void ) {

	szachownica = 0;
	glcd_drawRect( 0, 6, 128, 29, 1 );
	glcd_fillRect( 1, 7, 126, 27, 0 );
	setCurrentFont( &DefaultFont5x8 );
	glcd_puts( 7, 10, "STA disconnected", 1 );
	glcd_display();

#if SIMPLE_CLIENT_EXAMPLE == 1
//	if( udp_client_task_handle != NULL ) {
//		ESP_LOGW(TAG, "UDP CLIENT TASK - DELETE from callback");
//		vTaskDelete( udp_client_task_handle );
//		udp_client_task_handle = NULL;
//	}
#endif

}








void app_main() {

	uart_set_baudrate( 0, 115200 );

	vTaskDelay( 100 );	// tylko �eby �atwiej prze��cza� si� na terminal przy starcie
	printf("\nREADY\n");

	/*........ konfiguracja pin�w dla diod LED ..................................*/
//	gpio_set_direction( LED1_GPIO, GPIO_MODE_OUTPUT );
//	gpio_set_level( LED1_GPIO, 1 );

#if SIMPLE_CLIENT_EXAMPLE == 0
	/* ```````` Inicjalizacja 1wire ```````````````````````````````````````````` */
	ow_init( 13 );
#endif

	/* ```````` Inicjalizacja I2C `````````````````````````````````````````````` */
	i2c_init( 0, 5, 4 );

	/* ```````` Inicjalizacja OLED ````````````````````````````````````````````` */
	glcd_init( 220 );

#if SIMPLE_CLIENT_EXAMPLE == 0
	/* ```````` Inicjalizacja kolejki na potrzeby UDP `````````````````````````` */
	xQueueClientUDP = xQueueCreate( 5, sizeof( TQUEUE_UDP_CLI_DATA ) );
#endif

	/* ```````` Inicjalizacja NVS `````````````````````````````````````````````` */
	nvs_flash_init();

	/* ```````` Inicjalizacja WiFi ````````````````````````````````````````````` */
//	mk_wifi_init( WIFI_MODE_APSTA, mk_got_ip_cb, mk_sta_disconnected_cb, mk_ap_join_cb, mk_ap_leave_cb  );
//	mk_wifi_init( WIFI_MODE_AP, NULL, NULL, mk_ap_join_cb, mk_ap_leave_cb );
	mk_wifi_init( WIFI_MODE_STA, mk_got_ip_cb, mk_sta_disconnected_cb, NULL, NULL );

	/* ```````` Skanowanie dost�pnych sieci  ``````````````````````````````````` */
	mk_wifi_scan( NULL );



	/*........ uruchamianie task�w roboczych ....................................*/



#if SIMPLE_CLIENT_EXAMPLE == 1

	while(1) {

		vTaskDelay( 100 );
	}


#else


	/* ---- zmienne lokalne g��wnego tasku ------------------------------------- */
	TQUEUE_UDP_CLI_DATA dane;

	char buf[128];

	uint8_t czujniki_cnt;
	uint8_t subzero, cel, cel_fract_bits;
	uint8_t sw=0;


	TickType_t xLastWakeTime = xTaskGetTickCount();


/*............ G��WNY W�TEK .....................*/
	while(1) {

		if( !sw ) {
			czujniki_cnt = search_sensors();
			DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
		}

		if( sw && czujniki_cnt > 0 ) {

			szachownica = 0;
			glcd_fillRect(  5, 40, 115, 14, 0 );

			setCurrentFont( &font_bitocra5x13 );

			int err = DS18X20_read_meas( gSensorIDs[0], &subzero, &cel, &cel_fract_bits );
			if( err == DS18X20_OK ) {
				int8_t acel = cel;
				if( subzero ) acel = cel * -1;
				sprintf( buf, "(1) Temperatura DS18B20: %d,%d C\n", acel, cel_fract_bits );
				printf( buf );

				sprintf( dane.data, "(1) Temperatura DS18B20: %d,%d C\n", acel, cel_fract_bits );

				xQueueSend( xQueueClientUDP, &dane, 0 );

				sprintf( buf, "(1)�   IN: %d.%d" "�C", acel, cel_fract_bits );
				glcd_puts( 5, 40, buf, 1 );

			} else {
				sprintf( buf, "(1)� error: %d", err );
				glcd_puts( 5, 40, buf, 1 );
				printf("(1) error: %d\n", err);

				sprintf( dane.data, "(1) error: %d\n", err );
				xQueueSend( xQueueClientUDP, &dane, 0 );
			}

			glcd_display();
		}

		if( !sw ) vTaskDelayUntil( &xLastWakeTime, 80 );

		sw ^= 1;
	}
#endif
}









