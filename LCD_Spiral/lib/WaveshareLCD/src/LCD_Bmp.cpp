/*****************************************************************************
* | File        :   LCD_Bmp.c
* | Author      :   Amit Resh, May 2025
* |                 
* | Function    :   Display BMP file pics on WaveShare LCD 
* | Info        :   Accepts BMP files with 24, 16 or 8 Bits/Pixel
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*---------------- Based On -------------
* | Waveshare team (Basic version)
* | version     :   V1.0
* | Date        :   2018-01-11
******************************************************************************/
// #include <SD.h>
// #include <SPI.h>

#include "LCD_Bmp.h"
#include "Debug.h"

#define BUFFPIXEL_X3(__val)    ( (__val) * 3)                 // BUFFPIXELx3
#define RGB24TORGB565(R,G,B) (( (R) >> 3 ) << 11 ) | (( (G) >> 2 ) << 5) | ( (B) >> 3)
#define RGB555TORGB565(L,H)  (((H)<<9) | (((L)&0xC0)<<1) | ((L)&0x1F))


/// @author Amit Resh May 2025
/// @brief Display a BMP on WaveShare LCD
/// @param bmpFile - FILE* to open BMP file (on ESP32 FileSystem, like SPIFFS)
/// @param xOffset - Leftmost column
/// @param yOffset - Top most line
void LCD_DrawBmp( File bmpFile, uint16_t xOffset, uint16_t yOffset, int8_t zoom)
{
  uint16_t bytePerPix = BMPhdr.bitCount/8;
  uint16_t rasterWidth = BMPhdr.Width*bytePerPix;
  if (rasterWidth%4)
    rasterWidth += 4-(rasterWidth%4);
  uint8_t rasterLine[rasterWidth];                       //Buffer for one full raster line

  uint32_t LUT[BMPhdr.ColorsUsed];
  bool ARGB_Format=true;

  if (BMPhdr.ColorsUsed) {
    bmpFile.seek(14+40);                                 //Skip Headers to Color Pallette
    bmpFile.readBytes((char*)LUT, BMPhdr.ColorsUsed*4);
    for (int i=0 ; i<BMPhdr.ColorsUsed; ++i)
      ARGB_Format &= (LUT[i]>>24)==0xFF;
    Serial.printf("Using a color palette Format: %s\n", ARGB_Format ? "ARGB":"BGR0");
  }

  bmpFile.seek( BMPhdr.dataOffset);

  for ( uint16_t line = 0; line < BMPhdr.Height*zoom; line+=zoom) {  //Loop for all Raster lines
    bmpFile.read(rasterLine, rasterWidth);

    //  ======  Copy BMP line to LCD   ===================================================
    for (int r=0 ; r<zoom; ++r) {
      uint16_t k;
      LCD_SetWindow( xOffset, line+yOffset+r, xOffset + BMPhdr.Width*zoom, line+yOffset+r);
      LCD_DC_DATA;
      LCD_CS_0;
      for (uint16_t pix=0 ; pix<BMPhdr.Width ; ++pix) {
        k=pix*bytePerPix;
        switch (bytePerPix) {
          case 1:
            if (BMPhdr.ColorsUsed) {                                      //This BMP using a color pallette?
              uint8_t lutIdx = rasterLine[k];
              if (ARGB_Format)                                            //Using ARGB:
                for (int i=0 ; i<zoom ; ++i)
                  SPI4W_Write_Word(RGB24TORGB565((LUT[lutIdx]>>16)&0xFF, (LUT[lutIdx]>>8)&0xFF, LUT[lutIdx]&0xFF));
              else                                                        //Using BGR0:
                for (int i=0 ; i<zoom ; ++i)
                  SPI4W_Write_Word(RGB24TORGB565((LUT[lutIdx]>>8)&0xFF, (LUT[lutIdx]>>16)&0xFF, (LUT[lutIdx]>>24)&0xFF));
            }                                                             //Using Monochrome (256 shades)
            else
              for (int i=0 ; i<zoom ; ++i)
                SPI4W_Write_Word(RGB24TORGB565(rasterLine[k], rasterLine[k], rasterLine[k]));
            break;
          case 2:
            for (int i=0 ; i<zoom ; ++i)
              SPI4W_Write_Word(RGB555TORGB565(rasterLine[k], rasterLine[k+1]));
            break;
          case 3:
            for (int i=0 ; i<zoom ; ++i)
              SPI4W_Write_Word(RGB24TORGB565(rasterLine[k + 2], rasterLine[k + 1], rasterLine[k]));
            break;
        }
      }
      LCD_CS_1;
    }
  }
}

/// @author Amit Resh June 2025
/// @brief Display a BMP on WaveShare LCD
/// @param BmpMF - a BMP Memory File object
/// @param xOffset - Leftmost column
/// @param yOffset - Top most line
void LCD_DrawBmp( BmpMF& BmpMF, uint16_t xOffset, uint16_t yOffset, int8_t zoom)
{
  uint16_t bytePerPix = BMPhdr.bitCount/8;
  uint16_t rasterWidth = BMPhdr.Width*bytePerPix;
  if (rasterWidth%4)
    rasterWidth += 4-(rasterWidth%4);
  uint8_t rasterLine[rasterWidth];                       //Buffer for one full raster line

  uint32_t LUT[BMPhdr.ColorsUsed];
  bool ARGB_Format=true;

  if (BMPhdr.ColorsUsed) {
    BmpMF.BmpMFseek(14+40);                                 //Skip Headers to Color Pallette
    BmpMF.BmpMFread((unsigned char*)LUT, BMPhdr.ColorsUsed*4);
    for (int i=0 ; i<BMPhdr.ColorsUsed; ++i)
      ARGB_Format &= (LUT[i]>>24)==0xFF;
    Serial.printf("Using a color palette Format: %s\n", ARGB_Format ? "ARGB":"BGR0");
  }

  BmpMF.BmpMFseek(BMPhdr.dataOffset);

  for ( uint16_t line = 0; line < BMPhdr.Height*zoom; line+=zoom) {  //Loop for all Raster lines
    BmpMF.BmpMFread(rasterLine, rasterWidth);

    //  ======  Copy BMP line to LCD   ===================================================
    for (int r=0 ; r<zoom; ++r) {
      uint16_t k;
      LCD_SetWindow( xOffset, line+yOffset+r, xOffset + BMPhdr.Width*zoom, line+yOffset+r);
      LCD_DC_DATA;
      LCD_CS_0;
      for (uint16_t pix=0 ; pix<BMPhdr.Width ; ++pix) {
        k=pix*bytePerPix;
        switch (bytePerPix) {
          case 1:
            if (BMPhdr.ColorsUsed) {                                      //This BMP using a color pallette?
              uint8_t lutIdx = rasterLine[k];
              if (ARGB_Format)                                            //Using ARGB:
                for (int i=0 ; i<zoom ; ++i)
                  SPI4W_Write_Word(RGB24TORGB565((LUT[lutIdx]>>16)&0xFF, (LUT[lutIdx]>>8)&0xFF, LUT[lutIdx]&0xFF));
              else                                                        //Using BGR0:
                for (int i=0 ; i<zoom ; ++i)
                  SPI4W_Write_Word(RGB24TORGB565((LUT[lutIdx]>>8)&0xFF, (LUT[lutIdx]>>16)&0xFF, (LUT[lutIdx]>>24)&0xFF));
            }                                                             //Using Monochrome (256 shades)
            else
              for (int i=0 ; i<zoom ; ++i)
                SPI4W_Write_Word(RGB24TORGB565(rasterLine[k], rasterLine[k], rasterLine[k]));
            break;
          case 2:
            for (int i=0 ; i<zoom ; ++i)
              SPI4W_Write_Word(RGB555TORGB565(rasterLine[k], rasterLine[k+1]));
            break;
          case 3:
            for (int i=0 ; i<zoom ; ++i)
              SPI4W_Write_Word(RGB24TORGB565(rasterLine[k + 2], rasterLine[k + 1], rasterLine[k]));
            break;
        }
      }
      LCD_CS_1;
    }
  }
}
