/************************************************************************/
/*                                                                      */
/*        Access Dallas 1-Wire Device with ATMEL AVRs                   */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                      danni@specs.de                                  */
/*                                                                      */
/* modified by Martin Thomas <eversmith@heizung-thomas.de> 9/2004       */
/************************************************************************/
/*
 * onewire.c
 *
 *  Created on: 2009-08-22
 *      modyfikacje: Miros³aw Kardaœ
 */
#include "freertos/FreeRTOS.h"


#include "onewire.h"

static uint8_t one_wire_pin_nr;

//#define PORT_ENTER_CRITICAL 	portENTER_CRITICAL()
//#define PORT_EXIT_CRITICAL 		portEXIT_CRITICAL()

#define ONE_WIRE_GPIO_HIGH()  {\
	gpio_set_direction( one_wire_pin_nr, GPIO_MODE_OUTPUT_OD );	\
    gpio_set_level( one_wire_pin_nr, 1 );	\
	}

#define ONE_WIRE_GPIO_LOW()  {\
	gpio_set_direction( one_wire_pin_nr, GPIO_MODE_OUTPUT_OD );	\
    gpio_set_level( one_wire_pin_nr, 0 );	\
	}

#define ONE_WIRE_GPIO_DIR_IN()  {\
    gpio_set_direction( one_wire_pin_nr, GPIO_MODE_DEF_INPUT );	\
	}


#define OW_GET_IN()   ( gpio_get_level( one_wire_pin_nr ) )
#define OW_OUT_LOW()  ( ONE_WIRE_GPIO_LOW() )
#define OW_OUT_HIGH() ( ONE_WIRE_GPIO_HIGH() )
#define OW_DIR_IN()   ( ONE_WIRE_GPIO_DIR_IN() )
#define OW_DIR_OUT()  ( ONE_WIRE_GPIO_LOW() )


void ow_init( uint8_t ow_pin_nr ) {

	one_wire_pin_nr = ow_pin_nr;

	gpio_config_t pin_cfg;
	pin_cfg.pin_bit_mask = (1ul << ow_pin_nr);
	pin_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
	pin_cfg.mode = GPIO_MODE_OUTPUT_OD;
	pin_cfg.intr_type = GPIO_INTR_DISABLE;
	gpio_config( &pin_cfg );

}



static inline uint8_t _onewire_wait_for_bus(gpio_num_t pin, int max_wait)
{
	uint8_t state;
    for (int i = 0; i < ((max_wait + 4) / 5); i++)
    {
        if (gpio_get_level(pin))
            break;
        ets_delay_us(5);
    }
    state = gpio_get_level(pin);
    // Wait an extra 1us to make sure the devices have an adequate recovery
    // time before we drive things low again.
    ets_delay_us(1);
    return state;
}



uint8_t  ow_input_pin_state()
{
	return OW_GET_IN();
}

void  ow_parasite_enable(void)
{
    OW_OUT_HIGH();
	OW_DIR_OUT();
}

void  ow_parasite_disable(void)
{
    OW_OUT_LOW();
	OW_DIR_IN();
}

uint8_t  ow_reset(void) {

	uint8_t err;



	OW_OUT_LOW(); // disable internal pull-up (maybe on from parasite)
	OW_DIR_OUT(); // pull OW-Pin low for 480us

	_onewire_wait_for_bus(one_wire_pin_nr, 250);

	ets_delay_us(480);



	PORT_ENTER_CRITICAL;
	// set Pin as input - wait for clients to pull low
	OW_DIR_IN(); // input

	ets_delay_us(66);
	err = OW_GET_IN();		// no presence detect
	// nobody pulled to low, still high


//	SREG=sreg; // sei()

	// after a delay the clients should release the line
	// and input-pin gets back to high due to pull-up-resistor
	ets_delay_us(480-66);
	if( OW_GET_IN() == 0 )		// short circuit
		err = 1;

	PORT_EXIT_CRITICAL;

	_onewire_wait_for_bus(one_wire_pin_nr, 410);

	return err;
}

/* Timing issue when using runtime-bus-selection (!OW_ONE_BUS):
   The master should sample at the end of the 15-slot after initiating
   the read-time-slot. The variable bus-settings need more
   cycles than the constant ones so the delays had to be shortened
   to achive a 15uS overall delay
   Setting/clearing a bit in I/O Register needs 1 cyle in OW_ONE_BUS
   but around 14 cyles in configureable bus (us-Delay is 4 cyles per uS) */
uint8_t  ow_bit_io( uint8_t b ) {

	_onewire_wait_for_bus(one_wire_pin_nr, 10);

	PORT_ENTER_CRITICAL;

	OW_DIR_OUT(); // drive bus low

	ets_delay_us(3); // Recovery-Time wuffwuff was 1
	if ( b ) OW_DIR_IN(); // if bit is 1 set bus high (by ext. pull-up)
	ets_delay_us(15);

	if( OW_GET_IN() == 0 ) b = 0;  // sample at end of read-timeslot

	ets_delay_us(60-15);
	OW_DIR_IN();

	PORT_EXIT_CRITICAL;

//	SREG=sreg; // sei();

	return b;
}


uint8_t  ow_byte_wr( uint8_t b )
{
	uint8_t i = 8, j;

	_onewire_wait_for_bus(one_wire_pin_nr, 10);

//	PORT_ENTER_CRITICAL;

	do {
		j = ow_bit_io( b & 1 );
		b >>= 1;
		if( j ) b |= 0x80;
	} while( --i );

//	PORT_EXIT_CRITICAL;

	return b;
}


uint8_t  ow_byte_rd( void )
{
  // read by sending 0xff (a dontcare?)
	uint8_t res = ow_byte_wr( 0xFF );

  return res;


}


uint8_t  ow_rom_search( uint8_t diff, uint8_t *id )
{
	uint8_t i, j, next_diff;
	uint8_t b;

	if( ow_reset() ) return OW_PRESENCE_ERR;	// error, no device found

	ow_byte_wr( OW_SEARCH_ROM );			// ROM search command
	next_diff = OW_LAST_DEVICE;			// unchanged on last device

	i = OW_ROMCODE_SIZE * 8;					// 8 bytes

	do {
		j = 8;					// 8 bits
		do {
			b = ow_bit_io( 1 );			// read bit
			if( ow_bit_io( 1 ) ) {			// read complement bit
				if( b )					// 11
				return OW_DATA_ERR;			// data error
			}
			else {
				if( !b ) {				// 00 = 2 devices
					if( diff > i || ((*id & 1) && diff != i) ) {
					b = 1;				// now 1
					next_diff = i;			// next pass 0
					}
				}
			}
			ow_bit_io( b );     			// write bit
			*id >>= 1;
			if( b ) *id |= 0x80;			// store bit

			i--;

		} while( --j );

		id++;					// next byte

	} while( i );

	return next_diff;				// to continue search
}


void  ow_command( uint8_t command, uint8_t *id )
{
	uint8_t i;

	ow_reset();

	if( id ) {
		ow_byte_wr( OW_MATCH_ROM );			// to a single device
		i = OW_ROMCODE_SIZE;
		do {
			ow_byte_wr( *id );
			id++;
		} while( --i );
	}
	else {
		ow_byte_wr( OW_SKIP_ROM );			// to all devices
	}

	ow_byte_wr( command );
}

