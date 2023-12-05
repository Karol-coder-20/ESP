/*
 * mk_fota_http.c
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
// #include "..\build\include\sdkconfig.h"

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

#include "mqtt.h"


//-----------------------------------------

//#include "mk_wifi.h"
//#include "mk_tools.h"

#include "mk_fota_http.h"


#define BUFFSIZE 1500
#define TEXT_BUFFSIZE 1024

typedef enum esp_ota_firm_state {
    ESP_OTA_INIT = 0,
    ESP_OTA_PREPARE,
    ESP_OTA_START,
    ESP_OTA_RECVED,
    ESP_OTA_FINISH,
} esp_ota_firm_state_t;

typedef struct esp_ota_firm {
    uint8_t             ota_num;
    uint8_t             update_ota_num;

    esp_ota_firm_state_t    state;

    size_t              content_len;

    size_t              read_bytes;
    size_t              write_bytes;

    size_t              ota_size;
    size_t              ota_offset;

    const char          *buf;
    size_t              bytes;
} esp_ota_firm_t;


/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
/*an packet receive buffer*/
static char text[BUFFSIZE + 1] = { 0 };
/* an image total length*/
static int binary_file_length = 0;
/*socket id*/
static int socket_id = -1;

/*read buffer by byte still delim ,return read bytes counts*/
static int read_until(const char *buffer, char delim, int len) {

//  /*TODO: delim check,buffer check,further: do an buffer length limited*/
    int i = 0;
    while (buffer[i] != delim && i < len) {
        ++i;
    }
    return i + 1;
}


static bool connect_to_http_server(void *pvParameter) {

    char * tcp_ip = ( char* ) pvParameter;

    ESP_LOGI(TAG, "Server IP: %s Server Port:%d", tcp_ip, TCP_REMOTE_PORT );
//    ESP_LOGI(TAG, "Server IP: %s Server Port:%d", TCP_REMOTE_ADDR, TCP_REMOTE_PORT );

    int  http_connect_flag = -1;
    struct sockaddr_in sock_info;

    socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socket_id == -1) {
        ESP_LOGE(TAG, "Create socket failed!");
        return false;
    }
    ESP_LOGI(TAG, "Socket created OK!");

    // set connect info
    memset(&sock_info, 0, sizeof(struct sockaddr_in));
//    sock_info.sin_addr.s_addr = inet_addr( TCP_REMOTE_ADDR );
	sock_info.sin_addr.s_addr = inet_addr( tcp_ip );
    sock_info.sin_family = AF_INET;
    sock_info.sin_port = htons( TCP_REMOTE_PORT );

    // connect to http server
    http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
    if (http_connect_flag == -1) {
        ESP_LOGE(TAG, "Connect to server failed! errno=%d", errno);
        close(socket_id);
        return false;
    } else {
        ESP_LOGI(TAG, "Connected to server");
        return true;
    }
    return false;
}

static void __attribute__((noreturn)) task_fatal_error() {

    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    mqtt_publish("/topic/pc", "Exiting task due to fatal error...");
    close(socket_id);
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

bool _esp_ota_firm_parse_http(esp_ota_firm_t *ota_firm, const char *text, size_t total_len, size_t *parse_len) {

    /* i means current position */
    int i = 0, i_read_len = 0;
    char *ptr = NULL, *ptr2 = NULL;
    char length_str[32];

    while (text[i] != 0 && i < total_len) {
        if (ota_firm->content_len == 0 && (ptr = (char *)strstr(text, "Content-Length")) != NULL) {
            ptr += 16;
            ptr2 = (char *)strstr(ptr, "\r\n");
            memset(length_str, 0, sizeof(length_str));
            memcpy(length_str, ptr, ptr2 - ptr);
            ota_firm->content_len = atoi(length_str);
            ota_firm->ota_size = ota_firm->content_len;
            ota_firm->ota_offset = 0;
            ESP_LOGI(TAG, "parse Content-Length:%d, ota_size %d", ota_firm->content_len, ota_firm->ota_size);
            mqtt_publish("/topic/pc", "parse Content-Length:%d, ota_size %d", ota_firm->content_len, ota_firm->ota_size);
        }

        i_read_len = read_until(&text[i], '\n', total_len - i);

        if (i_read_len > total_len - i) {
            ESP_LOGE(TAG, "recv malformed http header");
            mqtt_publish("/topic/pc", "recv malformed http header");
            task_fatal_error();
        }

        // if resolve \r\n line, http header is finished
        if (i_read_len == 2) {
            if (ota_firm->content_len == 0) {
                ESP_LOGE(TAG, "did not parse Content-Length item");
                mqtt_publish("/topic/pc", "did not parse Content-Length item");
                task_fatal_error();
            }

            *parse_len = i + 2;

            return true;
        }

        i += i_read_len;
    }

    return false;
}

static size_t esp_ota_firm_do_parse_msg(esp_ota_firm_t *ota_firm, const char *in_buf, size_t in_len) {

    size_t tmp;
    size_t parsed_bytes = in_len;

    switch (ota_firm->state) {
        case ESP_OTA_INIT:
            if (_esp_ota_firm_parse_http(ota_firm, in_buf, in_len, &tmp)) {
                ota_firm->state = ESP_OTA_PREPARE;
                ESP_LOGD(TAG, "Http parse %d bytes", tmp);
                mqtt_publish("/topic/pc", "Http parse %d bytes", tmp);
                parsed_bytes = tmp;
            }
            break;
        case ESP_OTA_PREPARE:
            ota_firm->read_bytes += in_len;

            if (ota_firm->read_bytes >= ota_firm->ota_offset) {
                ota_firm->buf = &in_buf[in_len - (ota_firm->read_bytes - ota_firm->ota_offset)];
                ota_firm->bytes = ota_firm->read_bytes - ota_firm->ota_offset;
                ota_firm->write_bytes += ota_firm->read_bytes - ota_firm->ota_offset;
                ota_firm->state = ESP_OTA_START;
                ESP_LOGD(TAG, "Receive %d bytes and start to update", ota_firm->read_bytes);
                mqtt_publish("/topic/pc", "Receive %d bytes and start to update", ota_firm->read_bytes);
                ESP_LOGD(TAG, "Write %d total %d", ota_firm->bytes, ota_firm->write_bytes);
                mqtt_publish("/topic/pc", "Write %d total %d", ota_firm->bytes, ota_firm->write_bytes);
            }

            break;
        case ESP_OTA_START:
            if (ota_firm->write_bytes + in_len > ota_firm->ota_size) {
                ota_firm->bytes = ota_firm->ota_size - ota_firm->write_bytes;
                ota_firm->state = ESP_OTA_RECVED;
            } else
                ota_firm->bytes = in_len;

            ota_firm->buf = in_buf;

            ota_firm->write_bytes += ota_firm->bytes;

            ESP_LOGD(TAG, "Write %d total %d", ota_firm->bytes, ota_firm->write_bytes);
//            mqtt_publish("/topic/pc", "Write %d total %d", ota_firm->bytes, ota_firm->write_bytes);

            break;
        case ESP_OTA_RECVED:
            parsed_bytes = 0;
            ota_firm->state = ESP_OTA_FINISH;
            break;
        default:
            parsed_bytes = 0;
            ESP_LOGD(TAG, "State is %d", ota_firm->state);
            mqtt_publish("/topic/pc", "State is %d", ota_firm->state);
            break;
    }

    return parsed_bytes;
}

static void esp_ota_firm_parse_msg(esp_ota_firm_t *ota_firm, const char *in_buf, size_t in_len) {

    size_t parse_bytes = 0;

    ESP_LOGD(TAG, "Input %d bytes", in_len);
//    mqtt_publish("/topic/pc", "Input %d bytes", in_len);

    do {
        size_t bytes = esp_ota_firm_do_parse_msg(ota_firm, in_buf + parse_bytes, in_len - parse_bytes);
        ESP_LOGD(TAG, "Parse %d bytes", bytes);
//        mqtt_publish("/topic/pc", "Parse %d bytes", bytes);
        if (bytes)
            parse_bytes += bytes;
    } while (parse_bytes != in_len);
}

static inline int esp_ota_firm_is_finished(esp_ota_firm_t *ota_firm) {

    return (ota_firm->state == ESP_OTA_FINISH || ota_firm->state == ESP_OTA_RECVED);
}

static inline int esp_ota_firm_can_write(esp_ota_firm_t *ota_firm) {

    return (ota_firm->state == ESP_OTA_START || ota_firm->state == ESP_OTA_RECVED);
}

static inline const char* esp_ota_firm_get_write_buf(esp_ota_firm_t *ota_firm) {

    return ota_firm->buf;
}

static inline size_t esp_ota_firm_get_write_bytes(esp_ota_firm_t *ota_firm) {

    return ota_firm->bytes;
}

static void esp_ota_firm_init(esp_ota_firm_t *ota_firm, const esp_partition_t *update_partition) {

    memset(ota_firm, 0, sizeof(esp_ota_firm_t));
    ota_firm->state = ESP_OTA_INIT;
    ota_firm->ota_num = get_ota_partition_count();
    ota_firm->update_ota_num = update_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_0;

    ESP_LOGI(TAG, "Total OTA number %d update to %d part", ota_firm->ota_num, ota_firm->update_ota_num);
    mqtt_publish("/topic/pc", "Total OTA number %d update to %d part", ota_firm->ota_num, ota_firm->update_ota_num);

}

void ota_example_task(void *pvParameter) {

    char * tcp_ip = ( char* ) pvParameter;
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA example... @ %p flash %s", ota_example_task, CONFIG_ESPTOOLPY_FLASHSIZE);
    mqtt_publish("/topic/pc", "Starting OTA example... @ %p flash %s", ota_example_task, CONFIG_ESPTOOLPY_FLASHSIZE);

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        mqtt_publish("/topic/pc", "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
        mqtt_publish("/topic/pc", "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);
    mqtt_publish("/topic/pc", "Running partition type %d subtype %d (offset 0x%08x)",
            running->type, running->subtype, running->address);
    /*connect to http server*/
    if (connect_to_http_server(tcp_ip)) {
        ESP_LOGI(TAG, "Connected to http server");
        mqtt_publish("/topic/pc", "Connected to http server");
    } else {
        ESP_LOGE(TAG, "Connect to http server failed!");
        mqtt_publish("/topic/pc", "Connect to http server failed!");
        task_fatal_error();
    }

    /*send GET request to http server*/

    const char * frormat_str =
        "GET /firmware HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
    	"Accept: */*\r\n\r\n";



    char *http_request = NULL;

//    int get_len = asprintf(&http_request, frormat_str, FOTA_FILENAME, TCP_REMOTE_ADDR, TCP_REMOTE_PORT);
    int get_len = asprintf(&http_request, frormat_str, tcp_ip, TCP_REMOTE_PORT);

    printf(http_request);
    if (get_len < 0) {
        ESP_LOGE(TAG, "Failed to allocate memory for GET request buffer");
        mqtt_publish("/topic/pc", "Failed to allocate memory for GET request buffer");
        task_fatal_error();
    }
    int res = send(socket_id, http_request, get_len, 0);
    free(http_request);
    http_request = NULL;

    if (res < 0) {
        ESP_LOGE(TAG, "Send GET request to server failed");
        mqtt_publish("/topic/pc", "Send GET request to server failed");
        task_fatal_error();
    } else {
        ESP_LOGI(TAG, "Send GET request to server succeeded");
        mqtt_publish("/topic/pc", "Send GET request to server succeeded");
    }

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    mqtt_publish("/topic/pc", "Writing to partition subtype %d at offset 0x%x",
            update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
        mqtt_publish("/topic/pc", "esp_ota_begin failed, error=%d", err);
        task_fatal_error();
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    mqtt_publish("/topic/pc", "esp_ota_begin succeeded");

    bool flag = true;
    esp_ota_firm_t ota_firm;

    esp_ota_firm_init(&ota_firm, update_partition);

    /*deal with all receive packet*/
    while (flag) {
        memset(text, 0, TEXT_BUFFSIZE);
        memset(ota_write_data, 0, BUFFSIZE);
        int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
        if (buff_len < 0) { /*receive error*/
            ESP_LOGE(TAG, "Error: receive data error! errno=%d", errno);
            mqtt_publish("/topic/pc", "Error: receive data error! errno=%d", errno);
            task_fatal_error();
        } else if (buff_len > 0) { /*deal with response body*/
            esp_ota_firm_parse_msg(&ota_firm, text, buff_len);

            if (!esp_ota_firm_can_write(&ota_firm))
                continue;

            memcpy(ota_write_data, esp_ota_firm_get_write_buf(&ota_firm), esp_ota_firm_get_write_bytes(&ota_firm));
            buff_len = esp_ota_firm_get_write_bytes(&ota_firm);

            err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
                mqtt_publish("/topic/pc", "Error: esp_ota_write failed! err=0x%x", err);
                task_fatal_error();
            }
            binary_file_length += buff_len;

            uint8_t pr = (binary_file_length*100ul) / ota_firm.content_len;
            printf( "[9;LL]" "Have written image - %d %%\n", pr );	// xxx: warto�� procentowa
            mqtt_publish("/topic/pc", "Have written image - %d %%\n", pr );



        } else if (buff_len == 0) {  /*packet over*/
            flag = false;
            ESP_LOGI(TAG, "Connection closed, all packets received");
            mqtt_publish("/topic/pc", "Connection closed, all packets received");
            close(socket_id);
        } else {
            ESP_LOGE(TAG, "Unexpected recv result");
            mqtt_publish("/topic/pc", "Unexpected recv result");
        }

        if (esp_ota_firm_is_finished(&ota_firm))
            break;
    }

    ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);
    mqtt_publish("/topic/pc", "Total Write binary data length : %d", binary_file_length);

    if (esp_ota_end(update_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        mqtt_publish("/topic/pc", "esp_ota_end failed!");
        task_fatal_error();
    }
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
        mqtt_publish("/topic/pc", "esp_ota_set_boot_partition failed! err=0x%x", err);
        task_fatal_error();
    }
    ESP_LOGI(TAG, "Prepare to restart system!");
    mqtt_publish("/topic/pc", "Prepare to restart system!");


    esp_restart();
    return ;
}

