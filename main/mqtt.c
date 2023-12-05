/*
 * mqtt.c
 *
 *  Created on: 10 paź 2023
 *      Author: kkohut
 */

// mqtt.c

#include "mqtt.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


command_handler_t command_handlers[MAX_COMMAND_HANDLERS];
int num_command_handlers = 0;

// ... pozostałe funkcje ...

// Rejestracja funkcji obsługi komendy
void mqtt_register_command_handler(const char *command, void (*handler)(const char *message)) {
    if (num_command_handlers < MAX_COMMAND_HANDLERS) {
        // Skopiuj komendę i przypisz funkcję obsługi
        command_handlers[num_command_handlers].command = command;
        command_handlers[num_command_handlers].handler = handler;
        num_command_handlers++;
    } else {
        // Obsłuż błąd - przekroczono maksymalną liczbę obsługiwanych komend
        printf("Error: Maximum number of command handlers exceeded\n");
    }
}



static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(mqtt_client, "/topic/esp8266", 0);
            break;
        case MQTT_EVENT_DATA:
            printf("Received data: %.*s\n", event->data_len, event->data);
            // Przeszukaj zarejestrowane funkcje obsługi
            for (int i = 0; i < num_command_handlers; i++) {
            	if (strncmp(event->data, command_handlers[i].command, strlen(command_handlers[i].command)) == 0) {
                    // Znaleziono pasującą komendę, wywołaj funkcję obsługi
                    command_handlers[i].handler(event->data);
                    break;  // Przerwij pętlę po znalezieniu pasującej komendy
                }
            }
            break;
        default:
            break;
    }


    return ESP_OK;
}

//
//void mqtt_init(void) {
//    esp_mqtt_client_config_t mqtt_cfg = {
//        .uri = "mqtt://192.168.4.2:1883",  // Adres serwera MQTT
//        .event_handle = mqtt_event_handler,
//    };
//
//    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
//    esp_mqtt_client_start(mqtt_client);
//}


void mqtt_init(void) {
    if (mqtt_client == NULL) {  // Inicjuje klienta MQTT tylko jeśli nie został jeszcze zainicjowany
        esp_mqtt_client_config_t mqtt_cfg = {
            .uri = "mqtt://192.168.4.2:1883",
            .event_handle = mqtt_event_handler,
        };

        mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    }
    esp_mqtt_client_start(mqtt_client);  // Rozpoczyna połączenie MQTT
}

void mqtt_stop(void) {
    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);  // Zatrzymuje klienta MQTT
    }
}

void mqtt_publish(const char *topic, const char *format, ...) {
	// Przygotuj bufor na wiadomość logu
	char log_message[128]; // Dostosuj rozmiar bufora do potrzeb

	// Rozpocznij analizę zmiennych argumentów
	va_list args;
	va_start(args, format);

	// Użyj vsnprintf do sformatowania wiadomości logu
	vsnprintf(log_message, sizeof(log_message), format, args);

	// Zakończ analizę zmiennych argumentów
	va_end(args);

	// Wywołaj funkcję mqtt_publish, aby opublikować log na określonym temacie MQTT
//	mqtt_publish(topic, log_message);
	int msg_id = esp_mqtt_client_publish(mqtt_client, topic, log_message, 0, 0, 0);
    if (msg_id > 0) {
        printf("Successfully published message, msg_id=%d\n", msg_id);
    }
}

