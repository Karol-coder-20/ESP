/*
 * mqtt.h
 *
 *  Created on: 10 paź 2023
 *      Author: kkohut
 */

#ifndef MAIN_MQTT_H_
#define MAIN_MQTT_H_

//#include "esp_mqtt.h"

#include "mqtt_client.h"

extern esp_mqtt_client_handle_t mqtt_client;


#define MAX_COMMAND_HANDLERS 10  // Maksymalna liczba obsługiwanych komend

typedef struct {
    const char *command;
    void (*handler)(const char *message);
} command_handler_t;

void mqtt_init(void);
void mqtt_stop(void);
void mqtt_publish(const char *topic, const char *format, ...);
void mqtt_register_command_handler(const char *command, void (*handler)(const char *message));


#endif /* MAIN_MQTT_H_ */
