/*
 * udp_comm.h
 *
 *  Created on: 16 cze 2023
 *      Author: kkohut
 */

#ifndef MAIN_UDP_COMM_H_
#define MAIN_UDP_COMM_H_

#define DEST_PORT 8888
#define LOCAL_PORT 7777
#define RX_MESSAGE_LENGTH ( 128u )
#define DEST_IP_ADDR "255.255.255.255"

#define COMMAND_LENGTH ( 3u )
#define MAX_IP_LENGTH ( 16u )

void decode_udp_message(void *pvParameters);
void receive_udp_message();
void send_udp_message(void *pvParameter);
void init_udp_communication();  // dodajemy funkcję inicjalizacji

#endif /* MAIN_UDP_COMM_H_ */
