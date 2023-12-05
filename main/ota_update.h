/*
 * ota_ipdate.h
 *
 *  Created on: 19 lip 2023
 *      Author: kkohut
 */

#ifndef MAIN_OTA_UPDATE_H_
#define MAIN_OTA_UPDATE_H_

void ota_task(void *pvParameter);
void tcp_server_task(void* pvParameters);
void http_get_task(void *pvParameter);
void download_file_task(void *pvParameters);
void tcp_client_task(void *pvParameters);

#endif /* MAIN_OTA_UPDATE_H_ */
