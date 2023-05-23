//2019-06-17 11:28:58  Pixel Factory by mirekk36   www.atnel.pl
//CooperBlack12pt.c



#include "mk_fonts.h"

//  Font data for Cooper Black 12pt

// Character bitmaps for Cooper Black 12pt
const uint8_t CooperBlack12ptBitmaps[]  =
{
     // @0   [A]   (12 px wide)
     0x0E, 0x00,  //   ....###.....    //
     0x0F, 0x00,  //   ....####....    //
     0x0F, 0x00,  //   ....####....    //
     0x1F, 0x80,  //   ...######...    //
     0x1B, 0x80,  //   ...##.###...    //
     0x1B, 0xC0,  //   ...##.####..    //
     0x3F, 0xC0,  //   ..########..    //
     0x3F, 0xC0,  //   ..########..    //
     0x61, 0xE0,  //   .##....####.    //
     0xF3, 0xF0,  //   ####..######    //
     0xF3, 0xF0,  //   ####..######    //

     // @22   [E]   (10 px wide)
     0xFF, 0x80,  //   #########.    //
     0xFF, 0x80,  //   #########.    //
     0x78, 0x80,  //   .####...#.    //
     0x7B, 0x00,  //   .####.##..    //
     0x7F, 0x00,  //   .#######..    //
     0x7F, 0x00,  //   .#######..    //
     0x7B, 0x00,  //   .####.##..    //
     0x7B, 0x40,  //   .####.##.#    //
     0x78, 0xC0,  //   .####...##    //
     0xFF, 0xC0,  //   ##########    //
     0xFF, 0x80,  //   #########.    //

     // @44   [L]   (10 px wide)
     0xFC, 0x00,  //   ######....    //
     0xFC, 0x00,  //   ######....    //
     0x78, 0x00,  //   .####.....    //
     0x78, 0x00,  //   .####.....    //
     0x78, 0x00,  //   .####.....    //
     0x78, 0x00,  //   .####.....    //
     0x78, 0x00,  //   .####.....    //
     0x78, 0xC0,  //   .####...##    //
     0x78, 0xC0,  //   .####...##    //
     0xFF, 0xC0,  //   ##########    //
     0xFF, 0x80,  //   #########.    //

     // @66   [N]   (12 px wide)
     0xF0, 0xF0,  //   ####....####    //
     0xF8, 0xF0,  //   #####...####    //
     0x7C, 0x60,  //   .#####...##.    //
     0x7E, 0x60,  //   .######..##.    //
     0x7F, 0x60,  //   .#######.##.    //
     0x7F, 0xE0,  //   .##########.    //
     0x67, 0xE0,  //   .##..######.    //
     0x63, 0xE0,  //   .##...#####.    //
     0x61, 0xE0,  //   .##....####.    //
     0xF0, 0xE0,  //   ####....###.    //
     0xF0, 0x60,  //   ####.....##.    //

     // @88   [T]   (12 px wide)
     0x7F, 0xE0,  //   .##########.    //
     0xFF, 0xF0,  //   ############    //
     0xEF, 0x70,  //   ###.####.###    //
     0xCF, 0x30,  //   ##..####..##    //
     0x0F, 0x00,  //   ....####....    //
     0x0F, 0x00,  //   ....####....    //
     0x0F, 0x00,  //   ....####....    //
     0x0F, 0x00,  //   ....####....    //
     0x0F, 0x00,  //   ....####....    //
     0x1F, 0x80,  //   ...######...    //
     0x1F, 0x80,  //   ...######...    //
};

// Character descriptors for Cooper Black 12pt
// { [Char width in bits], [Offset into CooperBlack12ptCharBitmaps in bytes] }
const FONT_CHAR_INFO CooperBlack12ptDescriptors[]  =
{
    { 12,0 },		// A  (65)
    { 0,0 },		// B
    { 0,0 },		// C
    { 0,0 },		// D
    { 10,22 },		// E  (69)
    { 0,0 },		// F
    { 0,0 },		// G
    { 0,0 },		// H
    { 0,0 },		// I
    { 0,0 },		// J
    { 0,0 },		// K
    { 10,44 },		// L  (76)
    { 0,0 },		// M
    { 12,66 },		// N  (78)
    { 0,0 },		// O
    { 0,0 },		// P
    { 0,0 },		// Q
    { 0,0 },		// R
    { 0,0 },		// S
    { 12,88 },		// T  (84)
};

// Font information for Cooper Black 12pt
const FONT_INFO CooperBlack12pt  =
{
    1,  // Down Space in pixels
    11,  // Character height
    'A',  //  Start character
    3,  //  number of pixels of interspace between characters
    0,  // number of pixels of space character
    CooperBlack12ptDescriptors, // Character descriptor array
    CooperBlack12ptBitmaps, // Character bitmap array
    0, // (Pointer) Font filename saved on SD card or 0 (null) otherwise
    1,// bits and byte orientation 0-T2B, 1-L2R
};
