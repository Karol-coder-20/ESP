#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include "esp_wifi.h"

esp_err_t initialize_wifi_as_sta(const char* ssid, const char* password);
esp_err_t initialize_wifi_as_ap(void);

#endif
