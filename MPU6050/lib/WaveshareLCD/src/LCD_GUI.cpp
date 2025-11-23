/*****************************************************************************
  | File      	:	LCD_GUI.c
  | Author      : Amit Resh
  | Date        : May 2025
  |
  | Function    :	Achieve drawing: draw points, lines, boxes, circles 
                    with: size, solid dotted line, solid rectangle, hollow
					          rectangle, solid circle, hollow circle.
                  Achieve display characters: Display a single character,
                    string, number
  |------------------------------------------------------------------------
  | based on    :   Waveshare team
  |	version     :   V1.0
  | Date        :   2017-08-16
  | Info        :   Basic version

******************************************************************************/
#include "LCD_GUI.h"
#include "Debug.h"

/******************************************************************************
function:	Swap Coordinates
parameter:
    Point1, Point2  :   Points to swap
******************************************************************************/
static void GUI_Swap(POINT& Point1, POINT& Point2)
{
  POINT Temp;
  Temp = Point1;
  Point1 = Point2;
  Point2 = Temp;
}

/******************************************************************************
function:	Clear Display
parameter:
  color - Background color
******************************************************************************/
void GUI_Clear(COLOR Color)
{
  LCD_Clear(Color);
}

/******************************************************************************
function:	Draw Point(Xpoint, Ypoint) Fill the color
parameter:
	Xpoint		  :   The x coordinate of the point
	Ypoint		  :   The y coordinate of the point
	Color		    :   Set color
	Dot_Pixel	  :	  point size
  DOT_STYLE   :   dot fill style 
******************************************************************************/
void GUI_DrawPoint(POINT Xpoint, POINT Ypoint, COLOR Color,
                   DOT_PIXEL Dot_Pixel, DOT_STYLE DOT_STYLE)
{
  if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page) {
    DEBUG("GUI_DrawPoint Input exceeds the normal display range\r\n");
    return;
  }

  int16_t XDir_Num , YDir_Num;
  if (DOT_STYLE == DOT_STYLE_DFT) {
    for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++) {
      for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) {
        LCD_SetPoint2Color(Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel, Color);
      }
    }
  } else {
    for (XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++) {
      for (YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) {
        LCD_SetPoint2Color(Xpoint + XDir_Num - 1, Ypoint + YDir_Num - 1, Color);
      }
    }
  }
}

/******************************************************************************
  function:	Draw a horizontal line
  parameter:
	Xstart  ：Starting x point coordinate
	Xend    ：End point x coordinat
	Y       ：y coordinate
	Color   ：The color of the line segment
  Line_Style  : LINE_SOLID | LINE_DOTTED
  Dot_Pixel   : Pixels per Dot [1X1] ... [8X8]
******************************************************************************/
static void GUI_DrawHorizontalLine(POINT Xstart, POINT Xend, POINT Y,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
  if (Line_Style==LINE_SOLID)
    LCD_SetArea2Color(Xstart, Y, Xend, Y+Dot_Pixel, Color);
  else {
    POINT x;
    for (x=Xstart ; x <= Xend-Dot_Pixel ; x += 2*Dot_Pixel)
      LCD_SetArea2Color(x, Y, x+Dot_Pixel, Y+Dot_Pixel, Color);

    if (x < Xend)
        LCD_SetArea2Color(x, Y, Xend, Y+Dot_Pixel, Color);
  }
}

/******************************************************************************
  function:	Draw a horizontal line
  parameter:
	X       ：X coordinate
	Ystart  ：Starting y point coordinate
	Yend    ：End point y coordinat
	Color   ：The color of the line segment
  Line_Style  : LINE_SOLID | LINE_DOTTED
  Dot_Pixel   : Pixels per Dot [1X1] ... [8X8]
******************************************************************************/
static void GUI_DrawVerticleLine(POINT X, POINT Ystart, POINT Yend,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
  if (Line_Style==LINE_SOLID) {
    LCD_SetArea2Color(X, Ystart, X+Dot_Pixel, Yend, Color);
  }
  else {
    POINT y;
    for (y=Ystart ; y<= Yend-Dot_Pixel ; y += 2*Dot_Pixel)
      LCD_SetArea2Color(X, y, X+Dot_Pixel, y+Dot_Pixel, Color);
    if (y < Yend)
        LCD_SetArea2Color(X, y, X+Dot_Pixel, Yend, Color);
  }
}

/******************************************************************************
  function:	Draw a line of arbitrary slope
  parameter:
	Xstart ：Starting x point coordinates
	Ystart ：Starting y point coordinates
	Xend   ：End point x coordinate
	Yend   ：End point y coordinate
	Color  ：The color of the line segment
  Line_Style  : LINE_SOLID | LINE_DOTTED
  Dot_Pixel   : Pixels per Dot [1X1] ... [8X8]
******************************************************************************/
void GUI_DrawLine(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
  if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page ||
      Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page) {
    DEBUG("GUI_DrawLine Input exceeds the normal display range\r\n");
    return;
  }

  if (Xstart > Xend)
    GUI_Swap(Xstart, Xend);
  if (Ystart > Yend)
    GUI_Swap(Ystart, Yend);

  if (Ystart==Yend) {
    GUI_DrawHorizontalLine(Xstart, Xend, Ystart, Color, Line_Style, Dot_Pixel);
    return;
  }

  if (Xstart==Xend) {
    GUI_DrawVerticleLine(Xstart, Ystart, Yend, Color, Line_Style, Dot_Pixel);
    return;
  }

  POINT Xpoint = Xstart;
  POINT Ypoint = Ystart;
  int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

  // Increment direction, 1 is positive, -1 is counter;
  int XAddway = Xstart < Xend ? 1 : -1;
  int YAddway = Ystart < Yend ? 1 : -1;

  //Cumulative error
  int Esp = dx + dy;
  char Line_Style_Temp = 0;

  for (;;) {
    Line_Style_Temp++;
    //Painted dotted line, 2 point is really virtual
    if (Line_Style == LINE_DOTTED && Line_Style_Temp % 3 == 0) {
      //DEBUG("LINE_DOTTED\r\n");
      GUI_DrawPoint(Xpoint, Ypoint, LCD_BACKGROUND, Dot_Pixel, DOT_STYLE_DFT);
      Line_Style_Temp = 0;
    } else {
      GUI_DrawPoint(Xpoint, Ypoint, Color, Dot_Pixel, DOT_STYLE_DFT);
    }
    if (2 * Esp >= dy) {
      if (Xpoint == Xend) break;
      Esp += dy;
      Xpoint += XAddway;
    }
    if (2 * Esp <= dx) {
      if (Ypoint == Yend) break;
      Esp += dx;
      Ypoint += YAddway;
    }
  }
}

/******************************************************************************
  function:	Draw a rectangle
  parameter:
	Xstart ：Rectangular  Starting x point coordinates
	Ystart ：Rectangular  Starting x point coordinates
	Xend   ：Rectangular  End point x coordinate
	Yend   ：Rectangular  End point y coordinate
	Color  ：The color of the Rectangular segment
	Filled : Whether it is filled--- 1 solid 0：empty
  Dot_Pixel   : Pixels per Dot [1X1] ... [8X8]
  Line_Style  : LINE_SOLID (default) | LINE_DOTTED -- irrelevant if Solid-Fill
******************************************************************************/
void GUI_DrawRectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                       COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel, LINE_STYLE Line_Style)
{
  if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page ||
      Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page) {
    DEBUG("Input exceeds the normal display range\r\n");
    return;
  }

  if (Xstart > Xend)
    GUI_Swap(Xstart, Xend);
  if (Ystart > Yend)
    GUI_Swap(Ystart, Yend);

  if (Filled ) {
  	LCD_SetArea2Color( Xstart, Ystart, Xend, Yend, Color);
  } else {
    GUI_DrawLine(Xstart, Ystart, Xend+Dot_Pixel, Ystart, Color, Line_Style, Dot_Pixel);
    GUI_DrawLine(Xstart, Ystart, Xstart, Yend+Dot_Pixel, Color, Line_Style, Dot_Pixel);
    GUI_DrawLine(Xend, Yend+Dot_Pixel, Xend, Ystart, Color, Line_Style, Dot_Pixel);
    GUI_DrawLine(Xend+Dot_Pixel, Yend, Xstart, Yend, Color, Line_Style, Dot_Pixel);
  }
}

/******************************************************************************
  function:	Use the 8-point method to draw a circle of the
				specified size at the specified position.
  parameter:
	X_Center  ：Center X coordinate
	Y_Center  ：Center Y coordinate
	Radius    ：circle Radius
	Color     ：The color of the ：circle segment
	Filled    : Whether it is filled: 1 filling 0：Do not
******************************************************************************/
void GUI_DrawCircle(POINT X_Center, POINT Y_Center, LENGTH Radius,
                    COLOR Color, DRAW_FILL  Draw_Fill , DOT_PIXEL Dot_Pixel)
{
  if (X_Center > sLCD_DIS.LCD_Dis_Column || Y_Center >= sLCD_DIS.LCD_Dis_Page) {
    DEBUG("GUI_DrawCircle Input exceeds the normal display range\r\n");
    return;
  }

  //Draw a circle from(0, R) as a starting point
  int16_t XCurrent, YCurrent;
  XCurrent = 0;
  YCurrent = Radius;

  //Cumulative error,judge the next point of the logo
  int16_t Esp = 3 - (Radius << 1 );

  int16_t sCountY;
  if (Draw_Fill == DRAW_FULL) {
    while (XCurrent <= YCurrent ) { //Realistic circles
      for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
        GUI_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//1
        GUI_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//2
        GUI_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//3
        GUI_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//4
        GUI_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//5
        GUI_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//6
        GUI_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );//7
        GUI_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );
      }
      if (Esp < 0 )
        Esp += 4 * XCurrent + 6;
      else {
        Esp += 10 + 4 * (XCurrent - YCurrent );
        YCurrent --;
      }
      XCurrent ++;
    }
  } else { //Draw a hollow circle
    while (XCurrent <= YCurrent ) {
      GUI_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//1
      GUI_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//2
      GUI_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//3
      GUI_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//4
      GUI_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//5
      GUI_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//6
      GUI_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//7
      GUI_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );//0

      if (Esp < 0 )
        Esp += 4 * XCurrent + 6;
      else {
        Esp += 10 + 4 * (XCurrent - YCurrent );
        YCurrent --;
      }
      XCurrent ++;
    }
  }
}

/******************************************************************************
  function:	Display ASCII character
  parameter:
	Xpoint           ：X coordinate
	Ypoint           ：Y coordinate
	Acsii_Char       ：ASCII of character
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the English character
	Color_Foreground : Select the foreground color of the English character
******************************************************************************/
void GUI_DisChar(POINT Xpoint, POINT Ypoint, const char Acsii_Char,
                 sFONT* Font, COLOR Color_Background, COLOR Color_Foreground)
{
  POINT Page, Column;

  if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page) {
    DEBUG("GUI_DisChar Input exceeds the normal display range\r\n");
    return;
  }

  uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
  const unsigned char *ptr = &Font->table[Char_Offset];

  for (Page = 0; Page < Font->Height; Page ++ ) {
    for (Column = 0; Column < Font->Width; Column ++ ) {

      //To determine whether the font background color and screen background color is consistent
      if (FONT_BACKGROUND == Color_Background) { //this process is to speed up the scan
        if (pgm_read_byte(ptr) & (0x80 >> (Column % 8)))
          GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
      } else {
        if (pgm_read_byte(ptr) & (0x80 >> (Column % 8))) {
          GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
        } else {
          GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
        }
      }
      //One pixel is 8 bits
      if (Column % 8 == 7)
        ptr++;
    }/* Write a line */
    if (Font->Width % 8 != 0)
      ptr++;
  }/* Write all */
}

/******************************************************************************
  function:	Display a string
  parameter:
	Xstart           ：X coordinate
	Ystart           ：Y coordinate
	pString          ：Pointer to char string to be displayed
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the English character
	Color_Foreground : Select the foreground color of the English character
******************************************************************************/
void GUI_DisString_EN(POINT Xstart, POINT Ystart, const char * pString,
                      sFONT* Font, COLOR Color_Background, COLOR Color_Foreground )
{
  POINT Xpoint = Xstart;
  POINT Ypoint = Ystart;

  if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page) {
    DEBUG("GUI_DisString_EN Input exceeds the normal display range\r\n");
    return;
  }

  while (* pString != '\0') {
    //if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y direction plus the height of the character
    if ((Xpoint + Font->Width ) > sLCD_DIS.LCD_Dis_Column ) {
      Xpoint = Xstart;
      Ypoint += Font->Height;
    }

    // If the Y direction is full, reposition to(Xstart, Ystart)
    if ((Ypoint  + Font->Height ) > sLCD_DIS.LCD_Dis_Page ) {
      Xpoint = Xstart;
      Ypoint = Ystart;
    }
    GUI_DisChar(Xpoint, Ypoint, * pString, Font, Color_Background, Color_Foreground);

    //The next character of the address
    pString ++;

    //The next word of the abscissa increases the font of the broadband
    Xpoint += Font->Width;
  }
}

/******************************************************************************
  function:	Display a (int) Number
  parameter:
	Xstart           ：X coordinate
	Ystart           : Y coordinate
	Nummber          : The number displayed
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the English character
	Color_Foreground : Select the foreground color of the English character
******************************************************************************/
#define  ARRAY_LEN 255
void GUI_DisNum(POINT Xpoint, POINT Ypoint, int32_t Nummber,
                sFONT* Font, COLOR Color_Background, COLOR Color_Foreground )
{

  int16_t Num_Bit = 0, Str_Bit = 0;
  uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
  uint8_t *pStr = Str_Array;

  if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page) {
    DEBUG("GUI_DisNum Input exceeds the normal display range\r\n");
    return;
  }

  //Converts a number to a string
  while (Nummber) {
    Num_Array[Num_Bit] = Nummber % 10 + '0';
    Num_Bit++;
    Nummber /= 10;
  }

  //The string is inverted
  while (Num_Bit > 0) {
    Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
    Str_Bit ++;
    Num_Bit --;
  }

  //show
  GUI_DisString_EN(Xpoint, Ypoint, (const char*)pStr, Font, Color_Background, Color_Foreground );
}



/******************************************************************************
  function:	Display the bit map,1 byte = 8bit = 8 points
  parameter:
	Xpoint ：X coordinate
	Ypoint : Y coordinate
	pMap   : Pointing to the picture
	Width  ：Bitmap Width
	Height : Bitmap Height
  note:
	This function is suitable for bitmap, because a 16-bit data accounted for 16 points
******************************************************************************/
void GUI_Disbitmap(POINT Xpoint, POINT Ypoint, const unsigned char *pMap,
                   POINT Width, POINT Height)
{
  POINT i, j, byteWidth = (Width + 7) / 8;
  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width; i ++) {
      if (*(pMap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        GUI_DrawPoint(Xpoint + i, Ypoint + j, WHITE, DOT_PIXEL_DFT, DOT_STYLE_DFT);
      }
    }
  }
}

/******************************************************************************
  function:	Display the Gray map,1 byte = 8bit = 2 points
  parameter:
	Xpoint ：X coordinate
	Ypoint : Y coordinate
	pMap   : Pointing to the picture
	Width  ：Bitmap Width
	Height : Bitmap Height
  note:
	This function is suitable for bitmap, because a 4-bit data accounted for 1 points
	Please use the Image2lcd generated array
******************************************************************************/
void GUI_DisGrayMap(POINT Xpoint, POINT Ypoint, const unsigned char *pBmp)
{
  //Get the Map header Gray, width, height
  char Gray;
  Gray = *(pBmp + 1);
  POINT Height, Width;
  Width = (*(pBmp + 3) << 8) | (*(pBmp + 2));
  Height = (*(pBmp + 5) << 8) | (*(pBmp + 4));

  POINT i, j;
  if (Gray == 0x04) { //Sixteen gray levels
    pBmp = pBmp + 6;
    for (j = 0; j < Height; j++)
      for (i = 0; i < Width / 2; i++) {
        GUI_DrawPoint(Xpoint + i * 2, Ypoint + j, ~(*pBmp >> 4), DOT_PIXEL_DFT, DOT_STYLE_DFT);
        GUI_DrawPoint(Xpoint + i * 2 + 1, Ypoint + j, ~*pBmp , DOT_PIXEL_DFT, DOT_STYLE_DFT);
        pBmp++;
      }
  } else {
    DEBUG("Does not support type\r\n");
    return;
  }
}

sFONT *GUI_GetFontSize(POINT Dx, POINT Dy)
{
  sFONT *Font = NULL;
  if (Dx > Font24.Width && Dy > Font24.Height) {
    Font = &Font24;
  } else if ((Dx > Font20.Width && Dx < Font24.Width) &&
             (Dy > Font20.Height && Dy < Font24.Height)) {
    Font = &Font20;
  } else if ((Dx > Font16.Width && Dx < Font20.Width) &&
             (Dy > Font16.Height && Dy < Font20.Height)) {
    Font = &Font16;
  } else if ((Dx > Font12.Width && Dx < Font16.Width) &&
             (Dy > Font12.Height && Dy < Font16.Height)) {
    Font = &Font12;
  } else if ((Dx > Font8.Width && Dx < Font12.Width) &&
             (Dy > Font8.Height && Dy < Font12.Height)) {
    Font = &Font8;
  } else {
    DEBUG("Please change the display area size, or add a larger font to modify\r\n");
  }
  return Font;
}
