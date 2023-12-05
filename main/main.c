///*
// * 	main.c
// *
// * 	autor: Karol Kohut
// */
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdbool.h>
//#include <string.h>
//#include <ctype.h>					// dla funkcji typu toupper() itp
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_system.h"
//#include "esp_spi_flash.h"
//
//#include "portmacro.h"
//#include "FreeRTOSConfig.h"
//#include "..\build\include\sdkconfig.h"
//
//#include "driver/gpio.h"
//#include "driver/uart.h"
//#include "driver/i2c.h"
//#include "freertos/event_groups.h"
//
//#include "esp_wifi.h"
//#include "esp_netif.h"
//#include "esp_event.h"
//#include "nvs.h"
//#include "nvs_flash.h"
//#include "tcpip_adapter.h"
//
//
//#include "lwip/sockets.h"
//#include "lwip/err.h"
//#include "lwip/sys.h"
//#include <lwip/netdb.h>
//#include <errno.h>
//
//
//#include "esp_err.h"
//#include "esp_log.h"
//
//#include "esp_ota_ops.h"
//#include "esp_app_format.h"
//
//
//#include "esp_http_client.h"
//#include "esp_https_ota.h"
//
//#include "esp_wifi_types.h"
//#include "esp_wifi.h"
//
//#include <time.h>
//#include "lwip/apps/sntp.h"
//
//#include "mk_fota_http.h"
//#include "mk_i2c.h"
//#include "vl53l1_api.h"
//
//#include "udp_comm.h"
//#include "ota_update.h"
//
//#include "sensor.h"
//
//
//#include "wifi_setup.h"
//#include "ota_update.h"
//
//#define WIFI_SSID "twoja_siec"
//#define WIFI_PASS "twoje_haslo"
//
//
//
//#define LED1_GPIO 		2
//const char* ssid = "ESP8266_KAJOJ";
//const char* password = "12345678";
//const int CONNECTED_BIT = BIT0;
//
//const char *TAG = "FOTA: ";
//
//#define DEST_IP_ADDR "255.255.255.255"
//#define DEST_PORT 8888
//#define LOCAL_PORT 7777
//
//
//#define TASK_PRIORITY 1
//#define TASK_STACK_SIZE 2048
//#define INTERVAL_SECONDS 1
//
//#define MESSAGE_LENGTH ( 26u )
//#define RX_MESSAGE_LENGTH ( 128u )
//
//#define COMMAND_LENGTH ( 3u )
//#define MAX_IP_LENGTH ( 16u )
//
//#define GPIO_INPUT_IO_14     14
//#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_14)
//#define ESP_INTR_FLAG_DEFAULT 0
//
//
//
//#define WIFI_SSID       "TwojAP"
//#define WIFI_PASSWORD   "TwojeHaslo"
//#define WIFI_CHANNEL    1
//#define WIFI_AUTH_MODE  WIFI_AUTH_WPA_WPA2_PSK
//
//typedef enum state {
//    INIT,
//    WIFI_CONNECTED,
//	WIFI_UPDATE,
//} state_t;
//
//TaskHandle_t xsend_udp_message;
//static EventGroupHandle_t s_wifi_event_group;
//static state_t current_state;
//static uint16_t distance = 0;
//
//typedef struct {
//    char *udp_msg;      // wskaźnik na ciąg znaków
//    size_t msg_length;  // długość ciągu znaków
//} udp_msg_t;
//// Definicja mutexu
//xSemaphoreHandle xMainSchedulerState_mutex = NULL;
////xSemaphoreHandle xSendViaUDP_semaphore = NULL;
//xSemaphoreHandle xDistance_semaphore = NULL;
//
//// Define the queue handle
////QueueHandle_t udp_queue;
////QueueHandle_t newIP_queue;
//
//
////xQueueHandle gpio_evt_queue = NULL;
//
//
//
//VL53L1_Dev_t dev1;
//VL53L1_DEV Dev1 = &dev1;
//
//VL53L1_Dev_t dev2;
//VL53L1_DEV Dev2 = &dev2;
//
//VL53L1_RangingMeasurementData_t data;
//
////static void send_udp_message(void *param);
//
//static void init_semaphores()
//{
//    xMainSchedulerState_mutex = xSemaphoreCreateMutex();
//    if (xMainSchedulerState_mutex == NULL) {
//        printf("Failed to create mutex\n");
//        // obsłuż błąd
//    }
//
//	xDistance_semaphore = xSemaphoreCreateBinary();
//	if (xDistance_semaphore == NULL) {
//		// Obsługa błędu niepowodzenia utworzenia semafora
//		// ...xSendViaUDP_semaphore
//		printf("Failed to xSendViaUDP_semaphore\n");
//	}
//
//	// Zwolnienie semafora
//	xSemaphoreGive(xDistance_semaphore);
//}
//
//static state_t current_state_get(void)
//{
//    state_t state;
//    xSemaphoreTake(xDistance_semaphore, portMAX_DELAY);
//    state = current_state;
//    xSemaphoreGive(xDistance_semaphore);
//    return state;
//}
//
//static state_t distance_get(void)
//{
//    uint16_t ret;
//    xSemaphoreTake(xDistance_semaphore, portMAX_DELAY);
//	ret = distance;
//    xSemaphoreGive(xDistance_semaphore);
//    return ret;
//}
//
//static void current_state_set(state_t param)
//{
//    xSemaphoreTake(xMainSchedulerState_mutex, portMAX_DELAY);
//    current_state = param;
//    xSemaphoreGive(xMainSchedulerState_mutex);
//}
//
//static void distance_set(uint16_t param)
//{
//    xSemaphoreTake(xMainSchedulerState_mutex, portMAX_DELAY);
//    distance = param;
//    xSemaphoreGive(xMainSchedulerState_mutex);
//}
//
//static esp_err_t event_handler(void *ctx, system_event_t *event)
//{
//
//	printf("EVENT:  ");
//		printf("%d",event->event_id);
//		printf("\n");
//    switch (event->event_id) {
//        case SYSTEM_EVENT_STA_START:
//            esp_wifi_connect();
//            ESP_LOGI(TAG,"TRY connect wifi");
//            break;
//        case SYSTEM_EVENT_STA_GOT_IP:
//            xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
//            break;
//        case SYSTEM_EVENT_STA_DISCONNECTED:
//        	current_state_set(INIT);
//            xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
//            break;
//        case SYSTEM_EVENT_STA_CONNECTED:
//        	current_state_set(WIFI_CONNECTED);
//			ESP_LOGI(TAG,"WIFI connected");
//
//			xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
//			break;
//        case SYSTEM_EVENT_AP_STACONNECTED:
//                    xTaskCreate(&ota_task, "ota_task", 4096, NULL, 5, NULL);
//                    break;
//        default:
//            break;
//    }
//    return ESP_OK;
//}
//
//void wifi_init()
//{
//    s_wifi_event_group = xEventGroupCreate();
//
//    tcpip_adapter_init();
//    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
//
//    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
//    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//
//    wifi_config_t wifi_config = {};
//    strcpy((char*)wifi_config.sta.ssid, ssid);
//    strcpy((char*)wifi_config.sta.password, password);
//    wifi_config.sta.bssid_set = false;
//
//    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
//    ESP_ERROR_CHECK(esp_wifi_start());
//}
//
//
////static void task_WIFI(void *pvParameters)
////{
////	tcpip_adapter_ip_info_t ip_info;
////	ip4_addr_t server_ip;
////	uint16_t u16ExecutiveFreq_ms;
////	bool bSendingInfo = true;
////	char message[ MESSAGE_LENGTH ] = { 0 };
////	char new_ip[ MAX_IP_LENGTH + 1 ] = { 0 };
////
////    while (1) {
////    	u16ExecutiveFreq_ms = 100u;
//////    	printf("DIS: %d\n", distance_get());
////        switch ( current_state_get() ) {
////        case INIT:
////        	if ( esp_wifi_connect() > 0 ) u16ExecutiveFreq_ms = 10000u;
////
////            break;
////        case WIFI_CONNECTED:
////
////			tcpip_adapter_get_ip_info( TCPIP_ADAPTER_IF_STA, &ip_info );
////
////			sprintf(message, "%s", ip4addr_ntoa(&ip_info.ip));
////			if ( bSendingInfo ) xTaskCreate( &send_udp_message, "send_udp_message_task", 4096, message, 5, NULL );
////
////        	u16ExecutiveFreq_ms = 5000u;
////
////
////        	break;
////        case WIFI_UPDATE:
////        	// Wait for a new message in the UDP queue
////        	xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "PODAJ IPSERWERA. \"+ip:xxx.xxx.xxx.xxx\"", 5, NULL);
////
////			if ( xQueueReceive(newIP_queue, new_ip, (TickType_t) 5000u / portTICK_PERIOD_MS) == pdPASS )
////			{
////				inet_pton(AF_INET, new_ip, &server_ip);
////
////				xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "STARTED_UPDATE", 5, NULL);
////
////				xTaskCreate(&ota_example_task, "ota_example_task", 8192, ip4addr_ntoa(&server_ip), 5, NULL);
////
////				vTaskDelay(pdMS_TO_TICKS(180000));
////				//if no reeset = FAILED
////				xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "UPDATED_FAILED", 5, NULL);
////
////				current_state_set(WIFI_CONNECTED);
////				break;
////			}
////
////
////		}
////        vTaskDelay(pdMS_TO_TICKS(INTERVAL_SECONDS * u16ExecutiveFreq_ms));
////
////    }
////}
//
//void sensor1_task(void)
//{
//	while(1)
//	{
//		printf("main range: %d \n ",get_distance(0));
//	}
//
//}
//
////esp_err_t event_handler(void *ctx, system_event_t *event) {
////    // W tym miejscu możemy obsłużyć różne zdarzenia związane z WiFi.
////    return ESP_OK;
////}
//
//void wifi_ap_task(void *pvParameter) {
//    esp_err_t ret;
//
//    // Inicjalizacja NVS (Non Volatile Storage)
//    ret = nvs_flash_init();
//    if (ret != ESP_OK) {
//        ESP_LOGE("AP_INIT", "Błąd inicjalizacji NVS: %d", ret);
//        vTaskDelete(NULL); // Zakończ to zadanie
//    }
//
//    // Inicjalizacja adaptera TCP/IP
//    tcpip_adapter_init();
//
//    // Inicjalizacja pętli zdarzeń
//    ret = esp_event_loop_init(event_handler, NULL);
//    if (ret != ESP_OK) {
//        ESP_LOGE("AP_INIT", "Błąd inicjalizacji pętli zdarzeń: %d", ret);
//        vTaskDelete(NULL); // Zakończ to zadanie
//    }
//
//    // Inicjalizacja konfiguracji WiFi
//    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//    ret = esp_wifi_init(&cfg);
//    if (ret != ESP_OK) {
//        ESP_LOGE("AP_INIT", "Błąd inicjalizacji WiFi: %d", ret);
//        vTaskDelete(NULL); // Zakończ to zadanie
//    }
//
//    // Ustawienie trybu AP
//    ret = esp_wifi_set_mode(WIFI_MODE_AP);
//    if (ret != ESP_OK) {
//        ESP_LOGE("AP_INIT", "Błąd ustawienia trybu WiFi na AP: %d", ret);
//        vTaskDelete(NULL); // Zakończ to zadanie
//    }
//
//    // Konfiguracja AP
//    wifi_config_t ap_config = {
//        .ap = {
//            .ssid = WIFI_SSID,
//            .ssid_len = strlen(WIFI_SSID),
//            .password = WIFI_PASSWORD,
//            .max_connection = 4,
//            .authmode = WIFI_AUTH_WPA2_PSK
//        }
//    };
//    if (strlen(WIFI_PASSWORD) == 0) {
//        ap_config.ap.authmode = WIFI_AUTH_OPEN;
//    }
//
//    ret = esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config);
//    if (ret != ESP_OK) {
//        ESP_LOGE("AP_INIT", "Błąd konfiguracji AP: %d", ret);
//        vTaskDelete(NULL); // Zakończ to zadanie
//    }
//
//    // Start WiFi
//    ret = esp_wifi_start();
//    if (ret != ESP_OK) {
//        ESP_LOGE("AP_INIT", "Błąd startu WiFi: %d", ret);
//        vTaskDelete(NULL); // Zakończ to zadanie
//    }
//
//    // Czekaj w nieskończoność (lub dodaj inne operacje w tym zadaniu)
//    while (1) {
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
//    }
//}
//void app_main() {
//
//	uart_set_baudrate( 0, 115200 );
//
//	vTaskDelay( 100 );	// tylko �eby �atwiej prze��cza� si� na terminal przy starcie
//	printf("\nREADY\n");
//
//	/*........ konfiguracja pin�w dla diod LED ..................................*/
//	gpio_set_direction( LED1_GPIO, GPIO_MODE_OUTPUT );
//	gpio_set_level( LED1_GPIO, 1 );
////    xTaskCreate(wifi_ap_task, "wifi_ap_task", 4096, NULL, 5, NULL);
//	ESP_ERROR_CHECK(initialize_wifi_as_sta("ESP", "12345678"));
////	 xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
////	ESP_ERROR_CHECK(initialize_wifi_as_ap());
////	xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
//
//	/* ```````` Inicjalizacja I2C `````````````````````````````````````````````` */
//
////	i2c_init( 0, 5, 4 );
//
//	/* ```````` Inicjalizacja NVS `````````````````````````````````````````````` */
////	nvs_flash_init();
//
//	/* ```````` Inicjalizacja sema `````````````````````````````````````````````` */
////	init_semaphores();
////
////	Sensor_pins_t pins = {14,16,13};
////	sensor_init(0, pins, 0x56);
//
//	/* ```````` Inicjalizacja WiFi ````````````````````````````````````````````` */
////	wifi_init();
//
//
////	current_state_set(INIT);
////
////	xTaskCreate(task_WIFI, "task_WIFI", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
////	xTaskCreate(sensor1_task, "sensor_task", 4096, NULL, 5, NULL);
//
//
//
//}
//
//
//
//
//
//
//
//
//
//
//#include "mqtt_client.h"
//#include <string.h>
//#include <stdio.h>
//#include <string.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_system.h"
//#include "nvs_flash.h"
//#include "esp_wifi.h"
//#include "driver/uart.h"
//#include "esp_log.h"
//#include "esp_http_server.h"
//#include "esp_http_client.h"
//#include "freertos/event_groups.h"
//#include "esp_ota_ops.h"
//#include "mk_fota_http.h"
//
//static esp_mqtt_client_handle_t mqtt_client;
//
//static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
//{
//    switch (event->event_id) {
//        case MQTT_EVENT_CONNECTED:
//            esp_mqtt_client_subscribe(mqtt_client, "/topic/esp8266", 0);
//            break;
//        case MQTT_EVENT_DATA:
//            printf("Received data: %.*s\n", event->data_len, event->data);
//            break;
//        default:
//            break;
//    }
//    return ESP_OK;
//}
//
//static esp_mqtt_client_config_t mqtt_cfg = {
//    .uri = "mqtt://192.168.0.213:1883",
//    .event_handle = mqtt_event_handler,
//};
//
//void app_main(void)
//{
//	    uart_set_baudrate(0, 115200);
//	    vTaskDelay(100); // tylko żeby łatwiej przełączać się na terminal przy starcie
//	    printf("\nREADY\n");
//
//	    nvs_flash_init();
//	    tcpip_adapter_init();
//	    esp_event_loop_init(NULL, NULL);
//    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
//    esp_mqtt_client_start(mqtt_client);
//}
//

#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "freertos/event_groups.h"
#include "esp_ota_ops.h"
#include "mk_fota_http.h"
#include "mqtt_client.h"
#include "mqtt.h"

#define WIFI_SSID "ESP8266_AP"
#define WIFI_PASS "12345678"
#define BUFFSIZE 1024


static char ota_write_data[BUFFSIZE + 1] = { 0 };
const char *TAG = "OTA";

esp_mqtt_client_handle_t mqtt_client;

void publish_task(void *pvParameter)
{
    char msg[20];
    int msg_id;

    while(1) {
//        sprintf(msg, "Hello");
//        msg_id = esp_mqtt_client_publish(mqtt_client, "/topic/esp8266", msg, 0, 0, 0);
//        if(msg_id > 0) {
//            printf("Successfully published message, msg_id=%d\n", msg_id);
//        }


        mqtt_publish("/topic/pc", "Hello from ESP8266!");
        vTaskDelay(1000 / portTICK_RATE_MS);  // Delay for 1 second
    }
}

//void app_main(void)
//{
//	    uart_set_baudrate(0, 115200);
//	    vTaskDelay(100); // tylko żeby łatwiej przełączać się na terminal przy starcie
//	    printf("\nREADY\n");
//
//	    nvs_flash_init();
//	    tcpip_adapter_init();
//	    esp_event_loop_init(NULL, NULL);
//    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
//    esp_mqtt_client_start(mqtt_client);
//}
//

#define WIFI_MAX 40
#define WIFI_ON 20
#define WIFI_OFF (WIFI_MAX - WIFI_ON)
typedef enum {
    AUTO = 0,
    ON,
    OFF
} wifi_update_mode_t;

wifi_update_mode_t current_mode = AUTO;  // Globalna zmienna przechowująca bieżący tryb działania
int wifi_counter = 0;
bool client_connected = false;

esp_err_t wifi_enable(void) {
    esp_err_t err = esp_wifi_start();
    if (err == ESP_OK) {
        printf("Wi-Fi Enabled\n");
    } else {
        printf("Error starting Wi-Fi: %d\n", err);
    }
    return err;
}

esp_err_t wifi_disable(void) {
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_OK) {
        printf("Wi-Fi Disabled\n");
    } else {
        printf("Error stopping Wi-Fi: %d\n", err);
    }
    return err;
}

void wifi_control_task(void *pvParameters) {
    bool is_wifi_on = false;  // Dodano zmienną śledzącą stan Wi-Fi
    while (1) {

        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Sprawdź stan co sekundę
        if (client_connected)
        {
        	continue;
        }

        wifi_update_mode_t mode = current_mode;

        if (mode == AUTO) {
            wifi_counter++;

            if (wifi_counter < WIFI_ON) {
            	if(!is_wifi_on){
					if (wifi_enable() == ESP_OK) {
						is_wifi_on = true;  // Aktualizacja stanu Wi-Fi
					}
            	}
            }

            else if (is_wifi_on) {
				if (wifi_disable() == ESP_OK) {
					is_wifi_on = false;  // Aktualizacja stanu Wi-Fi
				}
                }
            if(wifi_counter > WIFI_MAX) {
                wifi_counter = 0;  // Resetuj licznik
            }
        } else if (mode == ON) {
            if (!is_wifi_on) {
                if (wifi_enable() == ESP_OK) {
                    is_wifi_on = true;  // Aktualizacja stanu Wi-Fi
                }
            }
        } else if (mode == OFF) {
            if (is_wifi_on) {
                if (wifi_disable() == ESP_OK) {
                    is_wifi_on = false;  // Aktualizacja stanu Wi-Fi
                }
            }
        }
    }
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI("HTTP", "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_CONNECTED");

            mqtt_init();
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI("HTTP", "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_HEADER");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
//            if (!esp_http_client_is_chunked_response(evt->client)) {
//                printf("%.*s", evt->data_len, (char*)evt->data);
//            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI("HTTP", "HTTP_EVENT_DISCONNECTED");
            mqtt_stop();
            break;
    }
    return ESP_OK;
}

// Handler for HTTP GET Request
esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char* resp_str = "Hello, World!";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    printf("start OTA\n");
    char *server_ip = "192.168.4.2"; // Przykładowy adres IP
    xTaskCreate(&ota_example_task, "ota_task", 8192, (void *) server_ip, 5, NULL);
    return ESP_OK;
}

void wifi_init_softap() {
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void http_get_task(void *pvParameters)
{
    while(1) {
        esp_http_client_handle_t client = (esp_http_client_handle_t) pvParameters;
        esp_http_client_perform(client);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void handle_update(const char *message) {

    printf("start OTA\n");
    char *server_ip = "192.168.4.2"; // Przykładowy adres IP
    xTaskCreate(&ota_example_task, "ota_task", 8192, (void *) server_ip, 5, NULL);
}


void handle_print(const char *message) {
    char *saveptr;  // Zmienna używana przez strtok_r do przechowywania stanu między wywołaniami
    strtok_r((char *)message, ":", &saveptr);  // Pierwsza część, przed ':'
    char *word = strtok_r(NULL, ":", &saveptr);  // Druga część, po ':'

    if (word != NULL) {
        printf("Received word: %s\n", word);
    } else {
        printf("Error: Message format incorrect or no word provided\n");
    }
}

static esp_err_t wifi_event_handler(void* ctx, system_event_t* event)
{
    tcpip_adapter_ip_info_t ip_info; // Deklaracja na początku funkcji
    wifi_sta_list_t sta_list;
    tcpip_adapter_sta_list_t tcpip_sta_list;
    switch(event->event_id) {
        case SYSTEM_EVENT_AP_STACONNECTED:
            // Klient połączył się z punktem dostępu
            printf("Station %x:%x:%x:%x:%x:%x join, AID=%d\n",
                   MAC2STR(event->event_info.sta_connected.mac),
                   event->event_info.sta_connected.aid);
            client_connected = true;
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            // Klient rozłączył się z punktem dostępu
            printf("Station %x:%x:%x:%x:%x:%x leave, AID=%d\n",
                   MAC2STR(event->event_info.sta_disconnected.mac),
                   event->event_info.sta_disconnected.aid);
            client_connected = false;
            break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            // Adres IP został przypisany do klienta
            ESP_LOGI(TAG, "Station assigned IP: " IPSTR,
                     IP2STR(&event->event_info.ap_staipassigned.ip));
            break;
        default:
            break;
    }
    return ESP_OK;
}


void app_main() {
    uart_set_baudrate(0, 115200);
    vTaskDelay(100); // tylko żeby łatwiej przełączać się na terminal przy starcie
    printf("\nREADY\n");

    nvs_flash_init();
    tcpip_adapter_init();
    // Rejestracja funkcji obsługi zdarzeń
	esp_event_loop_init(wifi_event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_init_softap();
    printf("SoftAP Initialized\n");

    esp_http_client_config_t config_h = {
        .url = "http://192.168.4.2:5000/",
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config_h);

    // Rejestracja funkcji obsługi komendy "update"
    mqtt_register_command_handler("update", handle_update);
    mqtt_register_command_handler("print", handle_print);
    xTaskCreate(&wifi_control_task, "wifi_control_task", 2048, NULL, 5, NULL);
}
