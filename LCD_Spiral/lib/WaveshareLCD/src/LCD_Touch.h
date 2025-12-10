/*****************************************************************************
  | File          :   LCD_Touch.cpp
  | Author        :   Amit Resh, Mar 2025
  |
  | Based on      :   Waveshare team
  |     version   :   V1.0
  |     Date      :   2017-08-16
  |     Info      :   Basic version
  |
  | Function    :   LCD Touch Pad Driver
  | Info        :
  ----------------
******************************************************************************/
#ifndef __LCD_TOUCH_H_
#define __LCD_TOUCH_H_

#include "WVSHR_Config.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"

#define TP_PRESS_DOWN           0x80
#define TP_PRESSED              0x40

#define	CALIBRATION_SCAN		true
	
//Touch screen structure
typedef struct {
	POINT Xpoint0;
	POINT Ypoint0;
	POINT Xpoint;
	POINT Ypoint;
	unsigned char chStatus;
	unsigned char chType;
	int iXoff;
	int iYoff;
	float fXfac;
	float fYfac;
	//Select the coordinates of the XPT2046 touch
	//  screen relative to what scan direction
	LCD_SCAN_DIR TP_Scan_Dir;
}TP_DEV;

//Brush structure
typedef struct{
	POINT Xpoint;
	POINT Ypoint;
	COLOR Color;
	DOT_PIXEL DotPixel; 
}TP_DRAW;

typedef struct {
	POINT Xpoint;
	POINT Ypoint;
} TP_COORDINATE;

void TP_Init(void);
void TP_Calibration(void);
unsigned char TP_Scan(bool calibration);

extern TP_DEV sTP_DEV;
extern TP_DRAW sTP_Draw;
#endif
