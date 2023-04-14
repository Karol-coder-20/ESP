/*                  e-gadget.header
 * mk_fonts.h
 *
 *  Created on: 2019-05-21
 *    Modyfied: 2019-06-19 09:28:21
 *      Author: Miros³aw Kardaœ
 *         web: www.atnel.pl
 *
 *	Library: MK GLCD  ver: 1.1a
 *
 * 	 Obs³ugiwane wyœwietlacze/chipsety/magistrale:
 * 
 * 	 1. OLED 128x64 - SSD1306 I2C/SPI
 * 	 2. OLED 128x32 - SSD1306 I2C/SPI
 * 	 3. COG 128x64  - ST7565R SPI
 * 	 4. NOKIA 84x48 - PCD8544 SPI
 * 	 5. COG 128x64  - UC1701x SPI
 * 
 * 	 Biblioteka "MK GLCD" wymaga do prawid³owej pracy
 * 	 jednej z dwóch bibliotek:
 * 
 * 	 1. MK_I2C		https://sklep.atnel.pl/pl/p/0581_0582-MK-I2C-AVR-Biblioteka-C/244
 * 	 2. MK_SPI		https://sklep.atnel.pl/pl/p/0567_0570-MK-SPI-AVR-Biblioteka-C-/241
 * 
 * 	 F_CPU: 1MHz - 24MHz
 * 	 MCU: all ATmega uC with minimum 2Kb RAM
 * 	
 * 	 Uwaga! przed kompilacj¹ nale¿y dodaæ opcjê w ustawieniach toolchaina
 * 	 C/C++ Build/Settings/AVR C Linker/General/Other Arguments
 * 	 -Wl,-gc-sections
 * 	
 *
 *
 */
#include "freertos/FreeRTOS.h"

#ifndef TFT_FONTX_H_
#define TFT_FONTX_H_


// ==========================================================================
// structure definition
// ==========================================================================

// This structure describes a single character's display information
typedef struct
{
	uint8_t widthBits;					// width, in bits (or pixels), of the character
	uint16_t offset;					// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array

} FONT_CHAR_INFO;



typedef struct {

	uint8_t down_space;
	uint8_t heightPixels;	// height, in pages (8 pixels), of the font's characters
	uint8_t startChar;	// the first character in the font (e.g. in charInfo and data)
	uint8_t interspacePixels;	// number of pixels of interspace between characters
	uint8_t spacePixels;	// number of pixels of space character
	const FONT_CHAR_INFO * charInfo;	// pointer to array of char information
	const uint8_t* data;		// pointer to generated array of character visual representation
    char * FontFileName;    // (Pointer) Font filename saved on SD card or 0 (null) otherwise

    uint8_t bitOrientation;  // bits and byte orientation 0-T2B, 1-L2R

} FONT_INFO;



extern const FONT_INFO DefaultFont5x8;
extern const FONT_INFO MkMINI3x6;
extern const FONT_INFO FixedSys5x12;





extern const FONT_INFO mkCyfryBIG;

extern const FONT_INFO mkCyfry1BIG;

extern const FONT_INFO StencilSTD25x32;

extern const FONT_INFO RockwellCondensed14x32;

extern const FONT_INFO RockwellCondensed14x18;

extern const FONT_INFO mkFontL2R;

extern const FONT_INFO Dosis36;

extern const FONT_INFO font_bitocra5x13;
//extern const FONT_INFO font_bitocra7x13;




#endif /* TFT_FONTX_H_ */















