/*****************************************************************************
 * | File        : WaveShareDemo.h
 * | Function    : Demo header using OOP LCD library
 *****************************************************************************/

#ifndef __WAVESHARE_DEMO_H
#define __WAVESHARE_DEMO_H

#include "WaveshareLCD.h"
#include "LCDTouch.h"

// Demo functions
void GUI_Showtime(WaveshareLCD& lcd, POINT xStart, POINT yStart,
                  POINT xEnd, POINT yEnd, DevTime* pTime, COLOR color);
void GUI_Show(WaveshareLCD& lcd);
void TP_Dialog(WaveshareLCD& lcd);
void TP_DrawBoard(WaveshareLCD& lcd, LCDTouch& touch);

#endif // __WAVESHARE_DEMO_H
