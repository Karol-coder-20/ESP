/*
 * mk_fota_http.h
 *
 *  Created on: 29 kwi 2022
 *      Author: Miros�aw Karda�
 */

#ifndef MAIN_MK_FOTA_HTTP_H_
#define MAIN_MK_FOTA_HTTP_H_


/* BIN firmware - filename to download */
#define FOTA_FILENAME		"esp8266_fota_http_rtos.bin"







/*:::: praca ::::*/
#ifdef PRACA

/*----- adres lokalny komputera ---------------*/
//#define TCP_REMOTE_ADDR		"192.168.1.188"

/*----- subdomena np test.atnel.pl ------------*/
#define	TCP_DOMENA			"test.atnel.pl"
#define TCP_REMOTE_ADDR		"94.152.54.109"

#define TCP_REMOTE_PORT		80
#endif


/*:::: dom ::::*/
//#ifdef DOM

/*----- adres lokalny komputera ---------------*/
#define TCP_REMOTE_ADDR		"192.168.4.2"

/*----- subdomena np test.atnel.pl ------------*/
//#define	TCP_DOMENA			"test.atnel.pl"
//#define TCP_REMOTE_ADDR		"94.152.54.109"

#define TCP_REMOTE_PORT		5000
//#endif

/*:::: telefon ::::*/
#ifdef TELEFON
#define TCP_REMOTE_ADDR		"192.168.0.213"
#define TCP_REMOTE_PORT		9999
#endif






extern const char *TAG;




extern void ota_example_task(void *pvParameter);





#endif /* MAIN_MK_FOTA_HTTP_H_ */
