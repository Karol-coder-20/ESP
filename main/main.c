/*
 * 	main.c
 *
 * 	autor: Karol Kohut
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

#include "esp_ota_ops.h"
#include "esp_app_format.h"


#include "esp_http_client.h"
#include "esp_https_ota.h"

#include <time.h>
#include "lwip/apps/sntp.h"

#include "mk_fota_http.h"
#include "mk_i2c.h"
#include "vl53l1_api.h"




#define LED1_GPIO 		2
const char* ssid = "ESP8266_KAJOJ";
const char* password = "12345678";
const int CONNECTED_BIT = BIT0;

const char *TAG = "FOTA: ";

#define DEST_IP_ADDR "255.255.255.255"
#define DEST_PORT 8888
#define LOCAL_PORT 7777


#define TASK_PRIORITY 1
#define TASK_STACK_SIZE 2048
#define INTERVAL_SECONDS 1

#define MESSAGE_LENGTH ( 26u )
#define RX_MESSAGE_LENGTH ( 128u )

#define COMMAND_LENGTH ( 3u )
#define MAX_IP_LENGTH ( 16u )

#define GPIO_INPUT_IO_14     14
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_14)
#define ESP_INTR_FLAG_DEFAULT 0

typedef enum state {
    INIT,
    WIFI_CONNECTED,
	WIFI_UPDATE,
} state_t;

TaskHandle_t xsend_udp_message;
static EventGroupHandle_t s_wifi_event_group;
static state_t current_state;
static uint16_t distance = 0;

typedef struct {
    char *udp_msg;      // wskaźnik na ciąg znaków
    size_t msg_length;  // długość ciągu znaków
} udp_msg_t;
// Definicja mutexu
xSemaphoreHandle xMainSchedulerState_mutex = NULL;
xSemaphoreHandle xSendViaUDP_semaphore = NULL;
xSemaphoreHandle xDistance_semaphore = NULL;

// Define the queue handle
QueueHandle_t udp_queue;
QueueHandle_t newIP_queue;


xQueueHandle gpio_evt_queue = NULL;



VL53L1_Dev_t dev1;
VL53L1_DEV Dev1 = &dev1;

VL53L1_Dev_t dev2;
VL53L1_DEV Dev2 = &dev2;

VL53L1_RangingMeasurementData_t data;

static void send_udp_message(void *param);

static void init_semaphores()
{
    xMainSchedulerState_mutex = xSemaphoreCreateMutex();
    if (xMainSchedulerState_mutex == NULL) {
        printf("Failed to create mutex\n");
        // obsłuż błąd
    }
    // Inicjalizacja semafora
    xSendViaUDP_semaphore = xSemaphoreCreateBinary();
	if (xSendViaUDP_semaphore == NULL) {
		// Obsługa błędu niepowodzenia utworzenia semafora
		// ...xSendViaUDP_semaphore
		printf("Failed to xSendViaUDP_semaphore\n");
	}

	// Zwolnienie semafora
	xSemaphoreGive(xSendViaUDP_semaphore);


	xDistance_semaphore = xSemaphoreCreateBinary();
	if (xDistance_semaphore == NULL) {
		// Obsługa błędu niepowodzenia utworzenia semafora
		// ...xSendViaUDP_semaphore
		printf("Failed to xSendViaUDP_semaphore\n");
	}

	// Zwolnienie semafora
	xSemaphoreGive(xDistance_semaphore);
}

static state_t current_state_get(void)
{
    state_t state;
    xSemaphoreTake(xDistance_semaphore, portMAX_DELAY);
    state = current_state;
    xSemaphoreGive(xDistance_semaphore);
    return state;
}

static state_t distance_get(void)
{
    uint16_t ret;
    xSemaphoreTake(xDistance_semaphore, portMAX_DELAY);
	ret = distance;
    xSemaphoreGive(xDistance_semaphore);
    return ret;
}

static void current_state_set(state_t param)
{
    xSemaphoreTake(xMainSchedulerState_mutex, portMAX_DELAY);
    current_state = param;
    xSemaphoreGive(xMainSchedulerState_mutex);
}

static void distance_set(uint16_t param)
{
    xSemaphoreTake(xMainSchedulerState_mutex, portMAX_DELAY);
    distance = param;
    xSemaphoreGive(xMainSchedulerState_mutex);
}

void decode_udp_message(void *pvParameters)
{
    char rx_buffer[RX_MESSAGE_LENGTH] = {0};

    while (1) {
        // Wait for a new message in the UDP queue
        if (xQueueReceive(udp_queue, rx_buffer, portMAX_DELAY) == pdPASS) {
            printf("Decoding message: %s\n", rx_buffer);

            // Split the message into tokens
            char *token;
            token = strtok(rx_buffer, " \t\n\r");

            if (token == NULL) {
                // Do nothing if the message is empty
            } else if (token[0] != '+') {
                // If the message doesn't start with a plus sign, check for valid commands

                if (strcmp(token, "idle") == 0) {
                    // Create a new task to send a UDP message
                    xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "WYSLALES IDLE", 5, NULL);

                } else if (strcmp(token, "update") == 0) {
                    // Set the current state and create a new task to send a UDP message
                    current_state_set(WIFI_UPDATE);
                    xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "start update", 5, NULL);

                } else {
                    // Create a new task to send a UDP message
                    xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "NIE ZNAM TEGO POLECENIA", 5, NULL);
                }

            } else {
                // If the message starts with a plus sign, check for valid commands

                // Extract the command from the message
                char command[COMMAND_LENGTH + 1] = {0};
                char new_ip[MAX_IP_LENGTH] = {0};
                strncpy(command, &token[1], COMMAND_LENGTH);

                if (strcmp(command, "ip:") == 0) {
                    // Create a new task to send the received message
                    xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, token, 5, NULL);

                    // Extract the IP address from the message
                    if (strlen(token) - 4u) {
                        strncpy(new_ip, &token[4], sizeof(new_ip));
                        // Create a new task to send the extracted IP address
                        xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, new_ip, 5, NULL);

                        // Add the IP address to the newIP queue
                        if (xQueueSend(newIP_queue, new_ip, 0) != pdPASS) {
                            printf("Failed to add message to newIP queue\n");
                        }

                    } else {
                        // Create a new task to send an error message
                        xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "BRAK IP", 5, NULL);
                    }

                } else {
                    // Create a new task to send an error message
                    xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "NIE ZNAM TEGO POLECENIA", 5, NULL);
                }
            }
        }
    }
}


static void receive_udp_message()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("Failed to create socket\n");
        return;
    }

    struct sockaddr_in destAddr = {0}; // inicjalizacja zerami
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_port = htons(LOCAL_PORT);

    if (bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0) {
        printf("Failed to bind socket\n");
        return;
    }

    while (1) {
        char rx_buffer[RX_MESSAGE_LENGTH] = {0}; // czyszczenie bufora
        socklen_t fromlen = sizeof(destAddr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&destAddr, &fromlen);
        if (len < 0) {
            printf("Failed to receive message\n");
        } else {
            printf("Received message: %s\n", rx_buffer);
            // Add the message to the UDP queue
			if (xQueueSend(udp_queue, rx_buffer, 0) != pdPASS) {
				printf("Failed to add message to UDP queue\n");
			}

        }
    }
}


static void send_udp_message(void *pvParameter) {
    char *msg = (char *)pvParameter;

    if (xSemaphoreTake( xSendViaUDP_semaphore, pdMS_TO_TICKS(100) ) == pdTRUE ) {
        int sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if (sock < 0) {
            printf( "Failed to create socket\n" );
            xSemaphoreGive(xSendViaUDP_semaphore);
            vTaskDelete( NULL );
        }

        // Set socket option to allow broadcast
        int broadcast_enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr(DEST_IP_ADDR);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(DEST_PORT);

        char message[strlen(msg) + 6];
        sprintf(message, "ESP: %s\n", msg);
        int sent_bytes = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (sent_bytes < 0) {
            printf("Failed to send message\n");
            xSemaphoreGive(xSendViaUDP_semaphore);
            vTaskDelete( NULL );
        }

        printf("Message sent successfully\n");
        close(sock);
        // Release semaphore
        xSemaphoreGive(xSendViaUDP_semaphore);
        // Delete this task
        vTaskDelete( NULL );
    } else {
        printf("Failed to take semaphore\n");
        vTaskDelete( NULL );
    }
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{

	printf("EVENT:  ");
		printf("%d",event->event_id);
		printf("\n");
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            ESP_LOGI(TAG,"TRY connect wifi");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
        	current_state_set(INIT);
            xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
        	current_state_set(WIFI_CONNECTED);
			ESP_LOGI(TAG,"WIFI connected");

			xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
			break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_init()
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    wifi_config.sta.bssid_set = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}


static void task_WIFI(void *pvParameters)
{
	tcpip_adapter_ip_info_t ip_info;
	ip4_addr_t server_ip;
	uint16_t u16ExecutiveFreq_ms;
	bool bSendingInfo = true;
	char message[ MESSAGE_LENGTH ] = { 0 };
	char new_ip[ MAX_IP_LENGTH + 1 ] = { 0 };

    while (1) {
    	u16ExecutiveFreq_ms = 100u;
//    	printf("DIS: %d\n", distance_get());
        switch ( current_state_get() ) {
        case INIT:
        	if ( esp_wifi_connect() > 0 ) u16ExecutiveFreq_ms = 10000u;

            break;
        case WIFI_CONNECTED:

			tcpip_adapter_get_ip_info( TCPIP_ADAPTER_IF_STA, &ip_info );

			sprintf(message, "%s", ip4addr_ntoa(&ip_info.ip));
			if ( bSendingInfo ) xTaskCreate( &send_udp_message, "send_udp_message_task", 4096, message, 5, NULL );

        	u16ExecutiveFreq_ms = 5000u;


        	break;
        case WIFI_UPDATE:
        	// Wait for a new message in the UDP queue
        	xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "PODAJ IPSERWERA. \"+ip:xxx.xxx.xxx.xxx\"", 5, NULL);

			if ( xQueueReceive(newIP_queue, new_ip, (TickType_t) 5000u / portTICK_PERIOD_MS) == pdPASS )
			{
				inet_pton(AF_INET, new_ip, &server_ip);

				xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "STARTED_UPDATE", 5, NULL);

				xTaskCreate(&ota_example_task, "ota_example_task", 8192, ip4addr_ntoa(&server_ip), 5, NULL);

				vTaskDelay(pdMS_TO_TICKS(180000));
				//if no reeset = FAILED
				xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, "UPDATED_FAILED", 5, NULL);

				current_state_set(WIFI_CONNECTED);
				break;
			}


		}
        vTaskDelay(pdMS_TO_TICKS(INTERVAL_SECONDS * u16ExecutiveFreq_ms));

    }
}

void sensors_init( VL53L1_DEV Dev )
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	uint8_t byteData = 0;
	uint16_t wordData;


	Status = VL53L1_DataInit(Dev);
	printf( "init: %d \n", Status);
	Status = VL53L1_StaticInit(Dev);
	printf( "init2: %d \n", Status);
	Status = VL53L1_SetDistanceMode(Dev, VL53L1_DISTANCEMODE_LONG);
	printf( "dis: %d \n", Status);
	Status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, 50000);
	printf( "budget: %d \n", Status);
	Status = VL53L1_SetInterMeasurementPeriodMilliSeconds(Dev, 100);
	printf( "period: %d \n", Status);
    Status = VL53L1_StartMeasurement(Dev);
    printf( "start: %d \n", Status);
}

void sensor_task(void *pvParameters)
{
	static VL53L1_RangingMeasurementData_t RangingData;
	VL53L1_Error status = VL53L1_ERROR_NONE;
//	Dev->i2c_slave_address = 0x52;
    uint8_t byteData = 0;
    uint16_t wordData;
    VL53L1_Dev_t dev;

    dev.i2c_slave_address = 0x52;

	Dev1->i2c_slave_address = 0x54;
	Dev2->i2c_slave_address = 0x52;

//	gpio_set_level( 13, 0 );
//	gpio_set_level( 12, 0 );
//	vTaskDelay(pdMS_TO_TICKS(100));
//
	gpio_set_level( 13, 1 );

	vTaskDelay(pdMS_TO_TICKS(100));

    VL53L1_RdByte(Dev1, 0x010F, &byteData);
	if ( byteData == 0 )
	{
		printf("need to set new addr! \n");
		status = VL53L1_SetDeviceAddress(&dev, 0x56);
			printf( "addr: %d \n", status);
	}
	else
	{
		printf("no need to set new addr! \n");
	}

	byteData = 0;


	gpio_set_level( 12, 1 );

	vTaskDelay(pdMS_TO_TICKS(100));
		 VL53L1_RdByte(Dev2, 0x010F, &byteData);
		 printf( "rdByte: %d \n", byteData);


	sensors_init(Dev2);
	sensors_init(Dev1);

while(1)
{

	vTaskDelay(pdMS_TO_TICKS(1000));
	status = VL53L1_GetRangingMeasurementData(Dev2, &RangingData);
	printf("%d,%d\n", RangingData.RangeStatus,
								  RangingData.RangeMilliMeter);

}





static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
//    printf("ISR\n");
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
	gpio_set_level( LED1_GPIO, 0 );

}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            VL53L1_ClearInterruptAndStartMeasurement(Dev1);
        }
    }
}

void app_main() {

	uart_set_baudrate( 0, 115200 );

	vTaskDelay( 100 );	// tylko �eby �atwiej prze��cza� si� na terminal przy starcie
	printf("\nREADY\n");

	/*........ konfiguracja pin�w dla diod LED ..................................*/
	gpio_set_direction( LED1_GPIO, GPIO_MODE_OUTPUT );
	gpio_set_level( LED1_GPIO, 1 );


	gpio_set_direction( 13, GPIO_MODE_OUTPUT );
	gpio_set_level( 13, 0 );
	gpio_set_direction( 12, GPIO_MODE_OUTPUT );
	gpio_set_level( 12, 0 );

//	gpio_set_direction( 14, GPIO_MODE_INPUT );
	gpio_config_t io_conf;
	    io_conf.intr_type = GPIO_INTR_NEGEDGE;
	    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
	    io_conf.mode = GPIO_MODE_INPUT;
	    io_conf.pull_up_en = 1;
	    gpio_config(&io_conf);
//
//
//
	    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	        xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
	        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	        gpio_isr_handler_add(GPIO_INPUT_IO_14, gpio_isr_handler, (void*) GPIO_INPUT_IO_14);
//		gpio_set_level( 13, 1 );

	/* ```````` Inicjalizacja I2C `````````````````````````````````````````````` */

	i2c_init( 0, 5, 4 );

	/* ```````` Inicjalizacja NVS `````````````````````````````````````````````` */
	nvs_flash_init();

	/* ```````` Inicjalizacja sema `````````````````````````````````````````````` */
	init_semaphores();

	// Create the queue with a length of 10 messages
	    udp_queue = xQueueCreate(10, sizeof(char[RX_MESSAGE_LENGTH]));
	    if (udp_queue == NULL) {
	        printf("Failed to create UDP queue\n");
	        return;
	    }
	    newIP_queue = xQueueCreate(10, sizeof(char[MAX_IP_LENGTH + 1]));
	    if (newIP_queue == NULL) {
	        printf("Failed to create UDP queue\n");
	        return;
	    }

	/* ```````` Inicjalizacja WiFi ````````````````````````````````````````````` */
	wifi_init();

	xTaskCreate(&receive_udp_message, "receive_udp_message", 2048, NULL, 5, NULL);
	xTaskCreate(&decode_udp_message, "decode_udp_message", 4096, NULL, 4, NULL);


	current_state_set(INIT);

	xTaskCreate(task_WIFI, "task_WIFI", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
	xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);


}


