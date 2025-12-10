/*****************************************************************************
* | File          :   LCD_Bmp.h
* | Author      :   Waveshare team
* | Function    :   Show SDcard BMP picto LCD 
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* | This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __LCD_BMP_H__
#define __LCD_BMP_H__

#include "WVSHR_Config.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "bmp.h"

#define MAX_BMP         10                      // bmp file num
#define FILENAME_LEN    20                      // max file name length

//  ======  BMP FILE FORMAT   ===========================================================================
//  ======  See: http://www.ue.eti.pg.gda.pl/fpgalab/zadania.spartan3/zad_vga_struktura_pliku_bmp_en.html

// +----------------------------+--------+-------------------------------+
// | Field                      | Bytes  | Description                   |
// +----------------------------+--------+-------------------------------+
// | **Bitmap File Header (14 bytes)**                                   |
// | Signature                  | 2      | 'BM' for Bitmap               |
// | File size                  | 4      | Total file size in bytes      |
// | Reserved                   | 4      | Reserved (usually 0)          |
// | Data offset                | 4      | Offset to start of pixel data |
// +----------------------------+--------+-------------------------------+
// | **DIB Header (40 bytes: BITMAPINFOHEADER)**                         |
// | Header size                | 4      | Size of this header (40)      |
// | Image width                | 4      | Width in pixels               |
// | Image height               | 4      | Height in pixels              |
// | Planes                     | 2      | Must be 1                     |
// | Bits per pixel             | 2      | 1, 4, 8, 16, 24, or 32        |
// | Compression                | 4      | 0 = none (BI_RGB)             |
// | Image size                 | 4      | Image data size (may be 0)    |
// | X pixels per meter         | 4      | Horizontal resolution         |
// | Y pixels per meter         | 4      | Vertical resolution           |
// | Colors in color table      | 4      | 0 = default                   |
// | Important color count      | 4      | 0 = all colors important      |
// +----------------------------+--------+-------------------------------+
// | **Color Table (only for 1, 4, or 8 bpp)**                           |
// | Palette entries            | 4 × n  | Each entry is 4 bytes BGR0 or |
// |                            |        |                          ARGB |
// +----------------------------+--------+-------------------------------+
// | **Pixel Data**                                                      |
// | Pixels                     | varies | Actual image data             |
// +----------------------------+--------+-------------------------------+


extern BMP_HDR BMPhdr;

void LCD_DrawBmp( File bmpFile, uint16_t xOffset, uint16_t yOffset, int8_t zoom=1);
void LCD_DrawBmp( BmpMF& BmpMF, uint16_t xOffset, uint16_t yOffset, int8_t zoom);

#endif
