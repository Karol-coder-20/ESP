/*
 * mk_i2c.c WRAPER - RTOS ESP8266
 *
 *  Created on: 12 mar 2022
 *      Author: Miros³aw Kardaœ
 *
 *	!!!    U W A G A    !!!
 *	nale¿y wprowadziæ poprawkê do oryginalnego pliku i2c.c w folderze:
 *
 *	\msys32\home\admin\ESP8266_RTOS\components\esp8266\driver
 *
 *	Jest to ORYGINALNY plik systemowy Espressif (i2c component) i zawiera
 *	paskudny b³¹d. B³¹d polega na tym, ¿e nie jest generowany sygna³ I2C STOP !!!
 *	gdy podczas wys³ania adresu urz¹dzenia I2C Slave na magistralê nie otrzymamy
 *	ACK (potwierdzenia) to tylko w tym wypadku nie jest generowany I2C STOP.
 *	Wiêkszoœæ uk³adów I2C radzi sobie z tym problemem, ale niektóre niestety nie
 *	i zaczynaj¹ dzia³aæ niepoprawnie. Skopana jest bowiem funkcja:
 *
 *	i2c_master_cmd_begin()
 *
 *	dlatego trzeba edytowaæ oryginalny plik i2c.h i znaleŸæ taki fragment kodu:
 *
 	     if (cmd->ack.en == 1) {			<--------- wystarczy wyszukaæ tê liniê
              if ((retVal & 0x01) != cmd->ack.exp) {
                    p_i2c->status = I2C_STATUS_ACK_ERROR;
                    return ;
              }
         }
 *
 * i przed return ; wprowadziæ kod do wygenerowania sygna³u I2C STOP jak ni¿ej
 *
 *
  	     if (cmd->ack.en == 1) {			<--------- wystarczy wyszukaæ tê liniê
              if ((retVal & 0x01) != cmd->ack.exp) {
                    p_i2c->status = I2C_STATUS_ACK_ERROR;

                    //........................ NAPRAWA sygna³u I2C STOP
                    i2c_master_wait(1);
                    i2c_master_set_dc(i2c_num, 0, i2c_last_state[i2c_num]->scl);
                    i2c_master_set_dc(i2c_num, 0, 1);
                    i2c_master_wait(2);     // sda 0, scl 1
                    i2c_master_set_dc(i2c_num, 1, 1);
                    i2c_master_wait(2);     // sda 1, scl 1

                    return ;
              }
         }
 *
 *  UWAGA! nale¿y o tym pamiêtaæ zawsze gdy na nowo wygenerowane zostanie œrodowisko
 *  RTOS dla ESP8266, albo gdy zostanie wgrane/rozpakowane nowe ale nie zawiera tej
 *  poprawki.
 *
 *  Gdy zapomni siê o tej poprawce - to niestety nowo albo na nowo skompilowane
 *  projekty mog¹ zacz¹æ dzia³aæ niepoprawnie.
 *
 *
 *
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_task_wdt.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/semphr.h"


#include "portmacro.h"
#include "FreeRTOSConfig.h"
#include "..\..\build\include\sdkconfig.h"

//------------------------------------------------

#include "mk_i2c.h"


#define I2CDEV_TIMEOUT		1000



#define MUTEX_ON		1



static const char *TAG = "mk_i2c";


static SemaphoreHandle_t i2c_mutex;














static esp_err_t i2c_create_mutex( void ) {

	i2c_mutex = xSemaphoreCreateMutex();
    if( !i2c_mutex )
    {
        ESP_LOGE(TAG, "Could not create device i2c_mutex");
        return ESP_FAIL;
    }

    return ESP_OK;
}


esp_err_t i2c_take_mutex( void ) {

#if MUTEX_ON == 1
    if( !xSemaphoreTake( i2c_mutex, pdMS_TO_TICKS( I2CDEV_TIMEOUT )) ) {

        ESP_LOGE(TAG, "Could not take i2c_mutex" );
        return ESP_ERR_TIMEOUT;
    }
#endif

    return ESP_OK;
}

esp_err_t i2c_give_mutex( void ) {

#if MUTEX_ON == 1
    if( !xSemaphoreGive( i2c_mutex ) ) {
        ESP_LOGE(TAG, "Could not give i2c_mutex" );
        return ESP_FAIL;
    }
#endif

    return ESP_OK;
}



esp_err_t  i2c_init( uint8_t port_nr, uint8_t scl_pin_nr, uint8_t sda_pin_nr ) {

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin_nr;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = scl_pin_nr;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.

    esp_err_t res = i2c_driver_install( port_nr, I2C_MODE_MASTER );
    if( res ) return res;
    res = i2c_param_config( port_nr, &conf );
    if( res ) return res;


    res = i2c_create_mutex();

    if( res ) return res;

    return ESP_OK;
}


esp_err_t i2c_check_dev( uint8_t port_nr, uint8_t slave_addr ) {

	I2C_TAKE_MUTEX;

    i2c_cmd_handle_t icmd = i2c_cmd_link_create();
	i2c_master_start( icmd );
	i2c_master_write_byte( icmd, slave_addr, 1 );
	i2c_master_stop( icmd );
	esp_err_t err = i2c_master_cmd_begin( 0, icmd, 100 );
    i2c_cmd_link_delete( icmd );

    I2C_GIVE_MUTEX;

	return err;

}



esp_err_t i2c_write_byte_to_dev( uint8_t port_nr, uint8_t slave_addr, uint8_t byte ) {

	I2C_TAKE_MUTEX;

    i2c_cmd_handle_t icmd = i2c_cmd_link_create();
	i2c_master_start( icmd );
	i2c_master_write_byte( icmd, slave_addr, 1 );
	i2c_master_write_byte( icmd, byte, 1 );
	i2c_master_stop( icmd );
	esp_err_t err = i2c_master_cmd_begin( 0, icmd, 100 );
    i2c_cmd_link_delete( icmd );

    I2C_GIVE_MUTEX;

	return err;
}


esp_err_t i2c_write_word_to_dev( uint8_t port_nr, uint8_t slave_addr, uint16_t word ) {

	I2C_TAKE_MUTEX;

    i2c_cmd_handle_t icmd = i2c_cmd_link_create();
	i2c_master_start( icmd );
	i2c_master_write_byte( icmd, slave_addr, 1 );
	i2c_master_write_byte( icmd, (word >> 8) , 1 );		// MSB
	i2c_master_write_byte( icmd, word & 0xFF, 1 );		// LSB
	i2c_master_stop( icmd );
	esp_err_t err = i2c_master_cmd_begin( 0, icmd, 100 );
    i2c_cmd_link_delete( icmd );

    I2C_GIVE_MUTEX;

	return err;
}



esp_err_t i2c_read_byte_from_dev( uint8_t port_nr, uint8_t slave_addr, uint8_t * byte ) {

	I2C_TAKE_MUTEX;

    i2c_cmd_handle_t icmd = i2c_cmd_link_create();
	i2c_master_start( icmd );
	i2c_master_write_byte( icmd, slave_addr | 1, 1 );
	i2c_master_read_byte( icmd, byte, 1 );
	i2c_master_stop( icmd );
	esp_err_t err = i2c_master_cmd_begin( 0, icmd, 100 );
    i2c_cmd_link_delete( icmd );

    I2C_GIVE_MUTEX;

	return err;
}


esp_err_t i2c_read_word_from_dev( uint8_t port_nr, uint8_t slave_addr, uint16_t * word ) {

	uint8_t msb, lsb;

	I2C_TAKE_MUTEX;

    i2c_cmd_handle_t icmd = i2c_cmd_link_create();
	i2c_master_start( icmd );
	i2c_master_write_byte( icmd, slave_addr | 1, 1 );
	i2c_master_read_byte( icmd, &msb, 0 );				// MSB
	i2c_master_read_byte( icmd, &lsb, 1 );				// LSB
	i2c_master_stop( icmd );
	esp_err_t err = i2c_master_cmd_begin( 0, icmd, 100 );
    i2c_cmd_link_delete( icmd );

    *word = (msb<<8) | lsb;

    I2C_GIVE_MUTEX;

	return err;
}




esp_err_t i2c_dev_read( uint8_t port_nr, uint8_t slave_addr,
		const void *out_data, size_t out_size,
		void *in_data, size_t in_size ) {

    if( !in_data || !in_size) return ESP_ERR_INVALID_ARG;

    I2C_TAKE_MUTEX;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if( out_data && out_size ) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, slave_addr, true);
        i2c_master_write(cmd, (void *)out_data, out_size, true);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slave_addr | 1, true);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(port_nr, cmd, pdMS_TO_TICKS(I2CDEV_TIMEOUT));
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d (%s)", slave_addr, port_nr, res, esp_err_to_name(res));

    i2c_cmd_link_delete(cmd);

    I2C_GIVE_MUTEX;

    return res;
}



esp_err_t i2c_dev_write( uint8_t port_nr, uint8_t slave_addr,
		const void *out_reg, size_t out_reg_size,
		const void *out_data, size_t out_size) {

    if( !out_data || !out_size ) return ESP_ERR_INVALID_ARG;


    I2C_TAKE_MUTEX;


    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slave_addr, true);

    if( out_reg && out_reg_size )
        i2c_master_write(cmd, (void *)out_reg, out_reg_size, true);

    i2c_master_write(cmd, (void *)out_data, out_size, true);
    i2c_master_stop(cmd);
    esp_err_t res = i2c_master_cmd_begin(port_nr, cmd, pdMS_TO_TICKS(I2CDEV_TIMEOUT));
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", slave_addr, port_nr, res, esp_err_to_name(res));
    i2c_cmd_link_delete(cmd);

    I2C_GIVE_MUTEX;

    return res;
}



esp_err_t i2c_dev_read_reg( uint8_t port_nr, uint8_t slave_addr, uint8_t reg,
        void *in_data, size_t in_size) {

    return i2c_dev_read( port_nr, slave_addr, &reg, 1, in_data, in_size);
}

esp_err_t i2c_dev_write_reg( uint8_t port_nr, uint8_t slave_addr, uint8_t reg,
        const void *out_data, size_t out_size) {

    return i2c_dev_write( port_nr, slave_addr, &reg, 1, out_data, out_size);
}












