#include "Arduino.h"
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
#include "LCD_Touch.h"
#include "Debug.h"
#include <stdlib.h>
#include <math.h>

TP_DEV sTP_DEV;
TP_DRAW sTP_Draw;
/*******************************************************************************
  function:
        Read the ADC of X/Y coordinates
  parameter:
    Channel_Cmd :   
                    
*******************************************************************************/
static TP_COORDINATE TP_Read_ADC(void)
{
  POINT Data = 0;
  TP_COORDINATE tp_xy;

  SPI.endTransaction();                                                       //Release SPI Semaphore
  SPI.beginTransaction(SPISettings(2 * 1000 * 1000, MSBFIRST, SPI_MODE0));    //Lower SPI clock

  TP_CS_0;

//  ======  X Ordinate  ====================================================================================================
  SPI4W_Write_Byte(0xd0);           //0xd0: Read channel x +, select the ADC resolution is 12 bits, set to differential mode
  Driver_Delay_us(200);

  Data = SPI4W_Read_Byte(0x00);
  Data <<= 8;//7bit
  Data |= SPI4W_Read_Byte(0x00);
  Data >>= 3;//5bit
  tp_xy.Xpoint = Data;

//  ======  Y Ordinate  ====================================================================================================
  SPI4W_Write_Byte(0x90);           //0x90: Read channel Y +, select the ADC resolution is 12 bits, set to differential mode
  Driver_Delay_us(200);

  Data = SPI4W_Read_Byte(0x00);
  Data <<= 8;//7bit
  Data |= SPI4W_Read_Byte(0x00);
  Data >>= 3;//5bit
  tp_xy.Ypoint = Data;

  TP_CS_1;

  SPI.endTransaction();                                                       //Release SPI Semaphore
  SPI.beginTransaction(SPISettings(8 * 1000 * 1000, MSBFIRST, SPI_MODE0));    //Restore SPI clock

  return tp_xy;
}

static void swapPoint(POINT& lft, POINT& rght)
{
  POINT tmp=lft;
  lft=rght;
  rght=tmp;
}

/*******************************************************************************
  function:
        Read 5 touch coordinate values and exclude the maximum and minimum
        returns the average for each ordinate
*******************************************************************************/
#define READ_TIMES  5   //Number of readings
static TP_COORDINATE TP_Read_ADC_Average(void)
{
  enum {XX=0, YY};
  TP_COORDINATE tp_xy;
  POINT Read_Buff[2][READ_TIMES];

//  ======  Read and save multiple samples for each Ordinate  =====================
  for (unsigned char i = 0; i < READ_TIMES; i++) {
    tp_xy = TP_Read_ADC();
    Read_Buff[XX][i] = tp_xy.Xpoint;
    Read_Buff[YY][i] = tp_xy.Ypoint;
    Driver_Delay_us(200);
  }

//  ======  Extract MIN/Max values for each Ordinate  =====================
  for (unsigned char ord=XX ; ord<=YY ; ++ord) {
    POINT min=Read_Buff[ord][0];
    unsigned char min_i=0;
    for (unsigned char i=1; i < READ_TIMES; i++)
      if (Read_Buff[ord][i] < min)
        min = Read_Buff[ord][min_i=i];
    swapPoint(Read_Buff[ord][0], Read_Buff[ord][min_i]);

    POINT max=Read_Buff[ord][1];
    unsigned char max_i=1;
    for (unsigned char i=2; i < READ_TIMES; i++)
      if (Read_Buff[ord][i] > max)
        max = Read_Buff[ord][max_i=i];
    swapPoint(Read_Buff[ord][READ_TIMES-1], Read_Buff[ord][max_i]);

    //Sum while excluding the largest and the smallest
    POINT Read_Sum = 0;
    for (unsigned char i = 1; i < READ_TIMES - 1; i ++)
      Read_Sum += Read_Buff[ord][i];

    //Average the rest
    Read_Buff[ord][0] = Read_Sum / (READ_TIMES - 2);
  }

  tp_xy.Xpoint = Read_Buff[XX][0];
  tp_xy.Ypoint = Read_Buff[YY][0];

  return tp_xy;
}

/*******************************************************************************
  function:
        Read X channel and Y channel AD value
  parameter:
    pXCh_Adc, pYCh_Adc  : Pointers to result coordinates
*******************************************************************************/
static TP_COORDINATE TP_Read_ADC_XY(void)
{
  TP_COORDINATE tp_xy = TP_Read_ADC_Average();    //Get touched coordinate
  return tp_xy;
}

/*******************************************************************************
  function:
        2 times to read the touch screen IC, and the two can not exceed the deviation,
        ERR_RANGE, meet the conditions, then that the correct reading, otherwise the reading error.
*******************************************************************************/
#define ERR_RANGE 50    //tolerance scope
static bool TP_Read_TwiceADC(POINT& XCh_Adc, POINT&  YCh_Adc )
{
  // POINT tp_xy1.Xpoint, tp_xy1.Ypoint, tp_xy2.Xpoint, tp_xy2.Ypoint;
  TP_COORDINATE tp_xy1, tp_xy2;

  //Read the ADC values Read the ADC values twice
  tp_xy1 = TP_Read_ADC_XY();
  //  Driver_Delay_us(10);
  tp_xy2 = TP_Read_ADC_XY();
  //  Driver_Delay_us(10);

  //The ADC error used twice is greater than ERR_RANGE to take the average
  if ( ((tp_xy2.Xpoint <= tp_xy1.Xpoint && tp_xy1.Xpoint < tp_xy2.Xpoint + ERR_RANGE) ||
        (tp_xy1.Xpoint <= tp_xy2.Xpoint && tp_xy2.Xpoint < tp_xy1.Xpoint + ERR_RANGE))
       && ((tp_xy2.Ypoint <= tp_xy1.Ypoint && tp_xy1.Ypoint < tp_xy2.Ypoint + ERR_RANGE) ||
           (tp_xy1.Ypoint <= tp_xy2.Ypoint && tp_xy2.Ypoint < tp_xy1.Ypoint + ERR_RANGE))) {
    XCh_Adc = (tp_xy1.Xpoint + tp_xy2.Xpoint) / 2;
    YCh_Adc = (tp_xy1.Ypoint + tp_xy2.Ypoint) / 2;
    return true;
  }

  //The ADC error used twice is less than ERR_RANGE returns failed
  return false;
}

/*******************************************************************************
  function:
        Calculation
  parameter:
        chCoordType:
                    1 : calibration
                    0 : relative position
*******************************************************************************/
unsigned char TP_Scan(bool calibration)
{
  //In X, Y coordinate measurement, IRQ is disabled and output is low
  if (!GET_TP_IRQ) {//Press the button to press
    //Read the physical coordinates
    if (calibration) {
      TP_Read_TwiceADC(sTP_DEV.Xpoint, sTP_DEV.Ypoint);                 //Just Read the screen coordinates
    }
    else {
      TP_Read_TwiceADC(sTP_DEV.Xpoint, sTP_DEV.Ypoint);
      //          DEBUG("(Xad,Yad) = %d,%d\r\n",sTP_DEV.Xpoint,sTP_DEV.Ypoint);
      if (sTP_DEV.TP_Scan_Dir == R2L_D2U) {       //Converts the result to screen coordinates
        sTP_Draw.Xpoint = sTP_DEV.fXfac * sTP_DEV.Xpoint +
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sTP_DEV.fYfac * sTP_DEV.Ypoint +
                          sTP_DEV.iYoff;
      } else if (sTP_DEV.TP_Scan_Dir == L2R_U2D) {
        sTP_Draw.Xpoint = sLCD_DIS.LCD_Dis_Column -
                          sTP_DEV.fXfac * sTP_DEV.Xpoint -
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sLCD_DIS.LCD_Dis_Page -
                          sTP_DEV.fYfac * sTP_DEV.Ypoint -
                          sTP_DEV.iYoff;
      } else if (sTP_DEV.TP_Scan_Dir == U2D_R2L) {
        sTP_Draw.Xpoint = sTP_DEV.fXfac * sTP_DEV.Ypoint +
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sTP_DEV.fYfac * sTP_DEV.Xpoint +
                          sTP_DEV.iYoff;
      } else {
        sTP_Draw.Xpoint = sLCD_DIS.LCD_Dis_Column -
                          sTP_DEV.fXfac * sTP_DEV.Ypoint -
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sLCD_DIS.LCD_Dis_Page -
                          sTP_DEV.fYfac * sTP_DEV.Xpoint -
                          sTP_DEV.iYoff;
      }
      //          DEBUG("( x , y ) = %d,%d\r\n",sTP_Draw.Xpoint,sTP_Draw.Ypoint);
    }
    if (0 == (sTP_DEV.chStatus & TP_PRESS_DOWN)) {  //Not being pressed
      sTP_DEV.chStatus = TP_PRESS_DOWN | TP_PRESSED;
      sTP_DEV.Xpoint0 = sTP_DEV.Xpoint;
      sTP_DEV.Ypoint0 = sTP_DEV.Ypoint;
    }
  } else {
    if (sTP_DEV.chStatus & TP_PRESS_DOWN) { //0x80
      sTP_DEV.chStatus &= ~(1 << 7);      //0x00
    } else {
      sTP_DEV.Xpoint0 = 0;
      sTP_DEV.Ypoint0 = 0;
      sTP_DEV.Xpoint = 0xffff;
      sTP_DEV.Ypoint = 0xffff;
    }
  }

  
  return (sTP_DEV.chStatus & TP_PRESS_DOWN);
}

/*******************************************************************************
  function:
        Use the default calibration factor
*******************************************************************************/
static void TP_GetAdFac(void)
{
  if ( sTP_DEV.TP_Scan_Dir == D2U_L2R ) { //SCAN_DIR_DFT = D2U_L2R
    sTP_DEV.fXfac = -0.132443F ;
    sTP_DEV.fYfac = 0.089997F ;
    sTP_DEV.iXoff = 516L ;
    sTP_DEV.iYoff = -22L ;
  } else if ( sTP_DEV.TP_Scan_Dir == L2R_U2D ) {
    sTP_DEV.fXfac = 0.089697F ;
    sTP_DEV.fYfac = 0.134792F ;
    sTP_DEV.iXoff = -21L ;
    sTP_DEV.iYoff = -39L ;
  } else if ( sTP_DEV.TP_Scan_Dir == R2L_D2U ) {
    sTP_DEV.fXfac = 0.089915F ;
    sTP_DEV.fYfac =  0.133178F ;
    sTP_DEV.iXoff = -22L ;
    sTP_DEV.iYoff = -38L ;
  } else if ( sTP_DEV.TP_Scan_Dir == U2D_R2L ) {
    sTP_DEV.fXfac = -0.132906F ;
    sTP_DEV.fYfac = 0.087964F ;
    sTP_DEV.iXoff = 517L ;
    sTP_DEV.iYoff = -20L ;
  } else {
    LCD_Clear(LCD_BACKGROUND);
    GUI_DisString_EN(0, 60, "Does not support touch-screen \
                        calibration in this direction",
                     &Font16, FONT_BACKGROUND, RED);
  }
}

/*******************************************************************************
  function:
        Touch pad initialization
*******************************************************************************/
void TP_Init(void)
{
  TP_CS_1;

  sTP_DEV.TP_Scan_Dir =  sLCD_DIS.LCD_Scan_Dir;
  // TP_Read_ADC_XY(sTP_DEV.Xpoint, sTP_DEV.Ypoint);

  TP_GetAdFac();
}



