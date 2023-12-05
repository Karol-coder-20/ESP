#include "wifi_setup.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include <string.h>

#include "ota_update.h"
#include "mk_fota_http.h"


#define WIFI_AP_SSID        "ESP8266_AP"
#define WIFI_AP_PASSWORD    "12345678"
#define MAX_WIFI_CONNECTIONS 4


static esp_err_t event_handler(void *ctx, system_event_t *event) {

	printf("EVENT:  ");
			printf("%d",event->event_id);
			printf("\n");
	    switch (event->event_id) {
	        case SYSTEM_EVENT_AP_STACONNECTED:
                    xTaskCreate(&ota_task, "ota_task", 4096, NULL, 5, NULL);
                    break;

	        case SYSTEM_EVENT_STA_START:
	               esp_wifi_connect();   // Tutaj dodajemy próbę połączenia.
	               break;
	        case SYSTEM_EVENT_STA_GOT_IP:


	        	xTaskCreate(ota_example_task, "OTA Task", 4096, "127.0.0.1", 5, NULL);
//	        	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
				   break;


        default:
            break;
	    }
    return ESP_OK;
}

esp_err_t initialize_wifi_as_sta(const char* ssid, const char* password) {
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    printf("startujemy \n\r");

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    return esp_wifi_start();
}

esp_err_t initialize_wifi_as_ap(void) {
    esp_err_t ret;

    // Inicjalizacja NVS
    ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE("AP_INIT", "Błąd inicjalizacji NVS: %d", ret);
        return ret;
    }

    // Inicjalizacja adaptera TCP/IP
    tcpip_adapter_init();

    // Inicjalizacja pętli zdarzeń
    ret = esp_event_loop_init(event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE("AP_INIT", "Błąd inicjalizacji pętli zdarzeń: %d", ret);
        return ret;
    }

    // Inicjalizacja konfiguracji WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE("AP_INIT", "Błąd inicjalizacji WiFi: %d", ret);
        return ret;
    }

    // Ustawienie trybu AP
    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret != ESP_OK) {
        ESP_LOGE("AP_INIT", "Błąd ustawienia trybu WiFi na AP: %d", ret);
        return ret;
    }

    // Konfiguracja AP
    wifi_config_t apConfig = {0};
    strcpy((char*)apConfig.ap.ssid, WIFI_AP_SSID);
    apConfig.ap.ssid_len = strlen(WIFI_AP_SSID);
    strcpy((char*)apConfig.ap.password, WIFI_AP_PASSWORD);
    apConfig.ap.max_connection = MAX_WIFI_CONNECTIONS;
    apConfig.ap.authmode = (strlen(WIFI_AP_PASSWORD) == 0) ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ret = esp_wifi_set_config(ESP_IF_WIFI_AP, &apConfig);
    if (ret != ESP_OK) {
        ESP_LOGE("AP_INIT", "Błąd konfiguracji AP: %d", ret);
        return ret;
    }

    // Start WiFi
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE("AP_INIT", "Błąd startu WiFi: %d", ret);
        return ret;
    }

    return ESP_OK;
}
