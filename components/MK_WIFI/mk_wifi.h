/*
 * mk_wifi.h
 *
 *  Created on: 5 kwi 2022
 *      Author: Miros�aw Karda�
 */

#ifndef COMPONENTS_MK_WIFI_MK_WIFI_H_
#define COMPONENTS_MK_WIFI_MK_WIFI_H_


/*----------------- USTAWIENIA STA ----------------------------*/

//#define DOM
//#define PRACA
#define TELEFON


#define MAXIMUM_RETRY  			0		// 0-infinite, maksymalna ilo�� pr�b ��czenia si� STA do AP
#define USE_STA_STATIC_IP		0		// 0-IP form DHCP, 1-Static IP


/*:::::::: praca ::::::::*/
#ifdef PRACA
#define STA_SSID      	"Atnel-2.4GHz"
#define STA_PASS      	"55AA55AA55"
#endif

/*:::::::: dom ::::::::*/
#ifdef DOM
#define STA_SSID      	"MirMUR"
#define STA_PASS      	"55AA55AA55"
#endif

/*:::::::: telefon ::::::::*/
#ifdef TELEFON
#define STA_SSID      	"HUAWEI"
#define STA_PASS      	"12345678"
#endif



#if USE_STA_STATIC_IP == 1

/*:::::::: dom ::::::::*/
#ifdef DOM
#define STA_IP		"192.168.2.170"
#define STA_GW		"192.168.2.2"
#define STA_MASK	"255.255.255.0"
#endif

/*:::::::: praca ::::::::*/
#ifdef PRACA
#define STA_IP		"192.168.1.170"
#define STA_GW		"192.168.1.1"
#define STA_MASK	"255.255.255.0"
#endif

/*:::::::: telefon ::::::::*/
#ifdef TELEFON
#define STA_IP		"10.0.0.2"
#define STA_GW		"10.0.0.1"
#define STA_MASK	"255.255.255.0"
#endif

#endif
/*----------------- USTAWIENIA STA KONIEC ---------------------*/



/*.................... USTAWIENIA AP ..............................*/
#define AP_SSID      	"ESP8266_MK"
#define AP_PASS      	"55AA55AA55"
#define AP_AUTH			WIFI_AUTH_WPA_WPA2_PSK
#define AP_MAX_STA_CONN		5


#define USE_AP_USER_IP			0	// 0-Default IP for AP, 1-User IP for AP


#if USE_AP_USER_IP == 1

/*:::::::: Set USER IP for AP ::::::::*/
#define AP_IP		"10.0.0.1"					// domy�lne ip: 192.168.4.1
#define AP_GW		"10.0.0.1"					// domy�lna brama: 192.168.4.1
#define AP_MASK		"255.255.255.0"				// domy�lna maska: 255.255.255.0

#endif
/*.................... USTAWIENIA AP KONIEC.......................*/



//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''









typedef void (*TSTA_GOT_IP_CB)( char * ip );
typedef void (*TSTA_DISCONNECTED)( void );
typedef void (*TAP_JOIN_CB)( char * mac );
typedef void (*TAP_LEAVE_CB)( char * mac );







/*'''''''''''''''' dost�pne funkcje ''''''''''''''''''''''''''''''*/

extern void mk_wifi_init( wifi_mode_t wifi_mode,
		TSTA_GOT_IP_CB got_ip_cb,
		TSTA_DISCONNECTED sta_discon_cb,
		TAP_JOIN_CB ap_join_cb,
		TAP_LEAVE_CB ap_leave_cb );





/*
 *	Sposoby u�ycia:
 *
 *	1.) Wyszukanie wszystkich dost�pnych AP w otoczeniu WIFI - mk_wifi_scan( NULL );
 *	2.) Wyszukanie/sprawdzenie konkretnego AP czy dost�pny:
 *
 *	uint8_t myssid[] = "MirMUR";
 *	mk_wifi_scan( myssid );
 *
 *	albo
 *
 *	mk_wifi_scan( (uint8_t*)"MirMUR" );
 *
 */
extern uint8_t mk_wifi_scan( uint8_t * assid );



#endif /* COMPONENTS_MK_WIFI_MK_WIFI_H_ */
