/*
 * udp_comm.c
 *
 *  Created on: 16 cze 2023
 *      Author: kkohut
 */

#include "udp_comm.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "portmacro.h"
// #include "..\build\include\sdkconfig.h"

#include "lwip/sockets.h"
void handle_idle_command();
void handle_update_command();

void handle_ip_command(char *ip);


QueueHandle_t udp_queue;

QueueHandle_t newIP_queue;

xSemaphoreHandle xSendViaUDP_semaphore = NULL;


typedef void (*CommandHandlerNoArg)();
typedef void (*CommandHandlerWithArg)(char*);

typedef enum { NO_ARG, WITH_ARG } CommandType;

typedef struct {
    char* command;
    CommandType cType;
    union {
        CommandHandlerNoArg noArgFunc;
        CommandHandlerWithArg withArgFunc;
    } handler;
} CommandMap;

CommandMap command_map[] = {
        {"idle", NO_ARG, .handler.noArgFunc = handle_idle_command},
        {"update", NO_ARG, .handler.noArgFunc = handle_update_command},
        {"ip:", WITH_ARG, .handler.withArgFunc = handle_ip_command},
        {NULL, NO_ARG, .handler.noArgFunc = NULL}  // End of array
    };

// Funkcja pomocnicza do tworzenia zadania wysyłania wiadomości UDP
void create_send_udp_message_task(char *message) {
    xTaskCreate(&send_udp_message, "send_udp_message_task", 4096, message, 5, NULL);
}

// Funkcje do obsługi poszczególnych poleceń
void handle_idle_command() {
    create_send_udp_message_task("WYSLALES IDLE");
}

void handle_update_command() {
    // current_state_set(WIFI_UPDATE);
    create_send_udp_message_task("start update");
}

void handle_ip_command(char *ip) {
    if (strlen(ip) - 3u) {
        char new_ip[MAX_IP_LENGTH] = {0};
        strncpy(new_ip, &ip[4], sizeof(new_ip));
        create_send_udp_message_task(new_ip);

        if (xQueueSend(newIP_queue, new_ip, 0) != pdPASS) {
            printf("Failed to add message to newIP queue\n");
        }
    } else {
        create_send_udp_message_task("BRAK IP");
    }
}

void decode_udp_message(void *pvParameters)
{
    char rx_buffer[RX_MESSAGE_LENGTH] = {0};

    while (1)
    {
        // Wait for a new message in the UDP queue
        if (xQueueReceive(udp_queue, rx_buffer, portMAX_DELAY) == pdPASS)
        {
            printf("Decoding message: %s\n", rx_buffer);

            // Split the message into tokens
            char *token = strtok(rx_buffer, " \t\n\r");

            if (token != NULL)
            {
				for (int i = 0; command_map[i].command != NULL; ++i)
				{
					if (strncmp(token, command_map[i].command, strlen(command_map[i].command)) == 0)
					{
						switch (command_map[i].cType)
						{
							case NO_ARG:
								command_map[i].handler.noArgFunc();
								break;
							case WITH_ARG:
								// przekazanie części tokena po poleceniu jako argument
								command_map[i].handler.withArgFunc(token);
								break;
						}
					break;
					}
				}
            }
		}
	}
}

void init_udp_communication()
{  // dodajemy funkcję inicjalizacji
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
    // Inicjalizacja innych potrzebnych rzeczy...

	// Inicjalizacja semafora
		xSendViaUDP_semaphore = xSemaphoreCreateBinary();
		if (xSendViaUDP_semaphore == NULL) {
			// Obsługa błędu niepowodzenia utworzenia semafora
			// ...xSendViaUDP_semaphore
			printf("Failed to xSendViaUDP_semaphore\n");
		}
		// Zwolnienie semafora
		xSemaphoreGive(xSendViaUDP_semaphore);



		xTaskCreate(&receive_udp_message, "receive_udp_message", 2048, NULL, 5, NULL);
		xTaskCreate(&decode_udp_message, "decode_udp_message", 4096, NULL, 4, NULL);
}

void receive_udp_message()
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


void send_udp_message(void *pvParameter) {
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

