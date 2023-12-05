/*
 * ota_update.c
 *
 *  Created on: 19 lip 2023
 *      Author: kkohut
 */

#include "ota_update.h"
#include "esp_ota_ops.h"
#include "lwip/sockets.h"
#include "lwip/err.h"

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_err.h"
#include "esp_http_client.h"

#define OTA_SERVER_IP "192.168.4.2"
#define OTA_SERVER_PORT 1234

#define SERVER_PORT 3333
#define BUFFSIZE 1024

esp_http_client_config_t config = {
    .url = "http://192.168.137.1:8000/esp8266_fota_http_rtos.bin",
};

/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
static const char *TAG = "HTTP_CLIENT";

#define SERVER_URL "http://192.168.137.1:8000/"

#define SERVER_IP "192.168.0.213" // Zmień na odpowiedni adres IP, jeśli jest inny
#define SERVER_PORT 80

//static const char *TAG = "TCP_CLIENT";

void tcp_client_task(void *pvParameters) {
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVER_PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Successfully connected");

    while (1) {
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        // Error occurred during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
            break;
        }
        // Data received
        else {
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            ESP_LOGI(TAG, "%s", rx_buffer);
        }
    }

    close(sock);
    ESP_LOGI(TAG, "Socket closed");
    vTaskDelete(NULL);
}

// Aby uruchomić ten task, użyj poniższego kodu w miejscu, w którym chcesz go zainicjować:
// xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);


esp_err_t http_event_handler(esp_http_client_event_t *evt) {
	printf("event: %d \n\r",evt->event_id);
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADERS_SENT");
            break;
//        case HTTP_EVENT_HEADER_SENT:
//            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
//            break;
        default:
            ESP_LOGW(TAG, "Unidentified event: %d", evt->event_id);
            break;
    }
    return ESP_OK;
}


void http_get_task(void *pvParameter) {
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        vTaskDelete(NULL);
        return;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET request successful");
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %d", err);
    }

    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}


void ota_task(void *pvParameter) {
    esp_err_t err;
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    bool update_success = false;

    // Inicjalizacja klienta HTTP
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE("OTA", "Failed to initialise HTTP connection");
        vTaskDelete(NULL);
        return;
    }

    // Wysłanie żądania GET do serwera, aby pobrać plik binarny
    err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE("OTA", "HTTP GET request failed: %d", err);
        esp_http_client_cleanup(client);
        vTaskDelete(NULL);
        return;
    }

    // Pobranie aktualnej partycji OTA
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    update_partition = esp_ota_get_next_update_partition(configured);
    if (!update_partition) {
        ESP_LOGE("OTA", "Failed to get update partition");
        esp_http_client_cleanup(client);
        vTaskDelete(NULL);
        return;
    }

    // Rozpoczęcie aktualizacji OTA
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE("OTA", "Failed to begin OTA.");
        esp_http_client_cleanup(client);
        vTaskDelete(NULL);
        return;
    }

    // Pobranie firmware'u i zapisanie w partycji OTA
    while (true) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        ESP_LOGI("OTA", "Read %d bytes from HTTP client", data_read);

        if (data_read < 0) {
            ESP_LOGE("OTA", "Error: SSL data read error");
            break;
        } else if (data_read == 0) {
            ESP_LOGI("OTA", "Connection closed, all data received");
            break;
        }

        err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
        if (err != ESP_OK) {
            ESP_LOGE("OTA", "Error: OTA write failed! err=0x%d", err);
            break;
        }
    }

    err = esp_ota_end(update_handle);
    if (err == ESP_OK) {
    	if (esp_ota_set_boot_partition(update_partition) == ESP_OK) {
    	            update_success = true;
    	        } else {
    	            ESP_LOGE("OTA", "Failed to set boot partition.");
    	        }
    } else {
        ESP_LOGE("OTA", "OTA end failed with error code: %d", err);
    }

    esp_http_client_cleanup(client);

    if (update_success) {
        ESP_LOGI("OTA", "Finished. Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE("OTA", "Failed to complete the OTA update.");
    }

    vTaskDelete(NULL);
}
