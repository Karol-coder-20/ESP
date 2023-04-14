/*
 * mk_i2c.h		RTOS ESP8266
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
 */

#ifndef COMPONENTS_MK_I2C_INCLUDE_MK_I2C_H_
#define COMPONENTS_MK_I2C_INCLUDE_MK_I2C_H_


//******** Wywo³ania Mutexów z funkcj¹ return ***************
#define I2C_TAKE_MUTEX do { \
		esp_err_t _err_ = i2c_take_mutex(); \
        if( _err_ != ESP_OK ) return _err_;\
    } while (0)

#define I2C_GIVE_MUTEX do { \
		esp_err_t _err_ = i2c_give_mutex(); \
        if (_err_ != ESP_OK) return _err_;\
    } while (0)


//******** Wywo³ania Mutexów bez funkcji return ***************
#define I2C_TAKE_MUTEX_NORET do { \
        i2c_take_mutex(); \
    } while (0)

#define I2C_GIVE_MUTEX_NORET do { \
        i2c_give_mutex(); \
    } while (0)






extern esp_err_t i2c_take_mutex( void );
extern esp_err_t i2c_give_mutex( void );





extern esp_err_t  i2c_init( uint8_t port_nr, uint8_t scl_pin_nr, uint8_t sda_pin_nr );

extern esp_err_t i2c_check_dev( uint8_t port_nr, uint8_t slave_addr );

extern esp_err_t i2c_write_byte_to_dev( uint8_t port_nr, uint8_t slave_addr, uint8_t byte );
extern esp_err_t i2c_write_word_to_dev( uint8_t port_nr, uint8_t slave_addr, uint16_t word );

extern esp_err_t i2c_read_byte_from_dev( uint8_t port_nr, uint8_t slave_addr, uint8_t * byte );
extern esp_err_t i2c_read_word_from_dev( uint8_t port_nr, uint8_t slave_addr, uint16_t * word );


extern esp_err_t i2c_dev_read( uint8_t port_nr, uint8_t slave_addr,
		const void *out_data, size_t out_size,
		void *in_data, size_t in_size );


extern esp_err_t i2c_dev_write( uint8_t port_nr, uint8_t slave_addr,
		const void *out_reg, size_t out_reg_size,
		const void *out_data, size_t out_size);


extern esp_err_t i2c_dev_read_reg( uint8_t port_nr, uint8_t slave_addr, uint8_t reg,
        void *in_data, size_t in_size);

extern esp_err_t i2c_dev_write_reg( uint8_t port_nr, uint8_t slave_addr, uint8_t reg,
        const void *out_data, size_t out_size);


#endif /* COMPONENTS_MK_I2C_INCLUDE_MK_I2C_H_ */
