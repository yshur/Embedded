#include <Arduino.h>
#include <stdlib.h>
#include <math.h>

#include "LCD_Touch.h"
#include "Debug.h"

/*******************************************************************************
  function:
        Draw Cross
  parameter:
            Xpoint :    The x coordinate of the point
            Ypoint :    The y coordinate of the point
            Color  :    Set color
*******************************************************************************/
static void TP_DrawCross(POINT Xpoint, POINT Ypoint, COLOR Color)
{
  GUI_DrawLine(Xpoint  -  12, Ypoint, Xpoint + 12, Ypoint,
               Color, LINE_SOLID, DOT_PIXEL_1X1);
  GUI_DrawLine(Xpoint, Ypoint  -  12, Xpoint, Ypoint + 12,
               Color, LINE_SOLID, DOT_PIXEL_1X1);
  GUI_DrawPoint(Xpoint, Ypoint, Color, DOT_PIXEL_2X2 , DOT_FILL_AROUND);
  GUI_DrawCircle(Xpoint, Ypoint, 6, Color, DRAW_EMPTY, DOT_PIXEL_1X1);
}

/*******************************************************************************
  function:
        The corresponding ADC value is displayed on the LC
  parameter:
            (Xpoint0 ,Xpoint0): The coordinates of the first point
            (Xpoint1 ,Xpoint1): The coordinates of the second point
            (Xpoint2 ,Xpoint2): The coordinates of the third point
            (Xpoint3 ,Xpoint3): The coordinates of the fourth point
            hwFac   :   Percentage of error
*******************************************************************************/
static void TP_ShowInfo(POINT Xpoint0, POINT Ypoint0,
                        POINT Xpoint1, POINT Ypoint1,
                        POINT Xpoint2, POINT Ypoint2,
                        POINT Xpoint3, POINT Ypoint3,
                        POINT hwFac)
{
  sFONT* TP_Font = &Font16;
  LENGTH TP_Dx =  TP_Font->Width;

  GUI_DrawRectangle(40, 160, 250, 270, WHITE, DRAW_FULL, DOT_PIXEL_1X1);

  GUI_DisString_EN(40, 160, "x1", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 160, "y1", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 180, "x2", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 180, "y2", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 200, "x3", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 200, "y3", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 220, "x4", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 220, "y4", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 240, "fac is : ", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 160, Xpoint0, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 160, Ypoint0, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 180, Xpoint1, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 180, Ypoint1, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 200, Xpoint2, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 200, Ypoint2, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 220, Xpoint3, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 220, Ypoint3, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 10 * TP_Dx , 240, hwFac, TP_Font, FONT_BACKGROUND, RED);
}

/*******************************************************************************
  function:
        Touch screen adjust
*******************************************************************************/
void TP_Calibration(void)
{
  unsigned char  cnt = 0;
  POINT XYpoint_Arr[4][2];
  uint32_t Dx, Dy;
  POINT Sqrt1, Sqrt2;
  float Dsqrt;

  LCD_Clear(LCD_BACKGROUND);
  GUI_DisString_EN(0, 60, "Please use the stylus click the cross"\
                   "on the screen. The cross will always move until"\
                   "the screen adjustment is completed.",
                   &Font16, FONT_BACKGROUND, RED);

  unsigned char Mar_Val = 12;
  TP_DrawCross(Mar_Val, Mar_Val, RED);

  sTP_DEV.chStatus = 0;

  while (1) {
    TP_Scan(CALIBRATION_SCAN);
    if ((sTP_DEV.chStatus & 0xC0) == TP_PRESSED) {
      sTP_DEV.chStatus &= ~(1 << 6);
      XYpoint_Arr[cnt][0] = sTP_DEV.Xpoint;
      XYpoint_Arr[cnt][1] = sTP_DEV.Ypoint;
      //          DEBUG("X%d,Y%d = %d,%d\r\n",
      //                cnt,cnt,
      //                XYpoint_Arr[cnt][0],
      //                XYpoint_Arr[cnt][1]);
      cnt ++;
      Driver_Delay_ms(200);

      switch (cnt) {
        case 1:
          TP_DrawCross(Mar_Val, Mar_Val, WHITE);
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val, Mar_Val, RED);
          Driver_Delay_ms(200);
          break;
        case 2:
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val, Mar_Val, WHITE);
          TP_DrawCross(Mar_Val, sLCD_DIS.LCD_Dis_Page - Mar_Val, RED);
          Driver_Delay_ms(200);
          break;
        case 3:
          TP_DrawCross(Mar_Val, sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                       sLCD_DIS.LCD_Dis_Page - Mar_Val, RED);
          Driver_Delay_ms(200);
          break;
        case 4:

          // 1.Compare the X direction
          Dx = abs((int16_t)(XYpoint_Arr[0][0] -
                             XYpoint_Arr[1][0]));//x1 - x2
          Dy = abs((int16_t)(XYpoint_Arr[0][1] -
                             XYpoint_Arr[1][1]));//y1 - y2
          Dx *= Dx;
          Dy *= Dy;
          Sqrt1 = sqrt(Dx + Dy);

          Dx = abs((int16_t)(XYpoint_Arr[2][0] -
                             XYpoint_Arr[3][0]));//x3 - x4
          Dy = abs((int16_t)(XYpoint_Arr[2][1] -
                             XYpoint_Arr[3][1]));//y3 - y4
          Dx *= Dx;
          Dy *= Dy;
          Sqrt2 = sqrt(Dx + Dy);

          Dsqrt = (float)Sqrt1 / Sqrt2;
          if (Dsqrt < 0.95 || Dsqrt > 1.05 || Sqrt1 == 0 || Sqrt2 == 0) {
            DEBUG("Adjust X direction \r\n");
            cnt = 0;
            TP_ShowInfo(XYpoint_Arr[0][0], XYpoint_Arr[0][1],
                        XYpoint_Arr[1][0], XYpoint_Arr[1][1],
                        XYpoint_Arr[2][0], XYpoint_Arr[2][1],
                        XYpoint_Arr[3][0], XYpoint_Arr[3][1],
                        Dsqrt * 100);
            Driver_Delay_ms(1000);
            TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                         sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
            TP_DrawCross(Mar_Val, Mar_Val, RED);
            continue;
          }

          // 2.Compare the Y direction
          Dx = abs((int16_t)(XYpoint_Arr[0][0] -
                             XYpoint_Arr[2][0]));//x1 - x3
          Dy = abs((int16_t)(XYpoint_Arr[0][1] -
                             XYpoint_Arr[2][1]));//y1 - y3
          Dx *= Dx;
          Dy *= Dy;
          Sqrt1 = sqrt(Dx + Dy);

          Dx = abs((int16_t)(XYpoint_Arr[1][0] -
                             XYpoint_Arr[3][0]));//x2 - x4
          Dy = abs((int16_t)(XYpoint_Arr[1][1] -
                             XYpoint_Arr[3][1]));//y2 - y4
          Dx *= Dx;
          Dy *= Dy;
          Sqrt2 = sqrt(Dx + Dy);//

          Dsqrt = (float)Sqrt1 / Sqrt2;
          if (Dsqrt < 0.95 || Dsqrt > 1.05) {
            DEBUG("Adjust Y direction \r\n");
            cnt = 0;
            TP_ShowInfo(XYpoint_Arr[0][0], XYpoint_Arr[0][1],
                        XYpoint_Arr[1][0], XYpoint_Arr[1][1],
                        XYpoint_Arr[2][0], XYpoint_Arr[2][1],
                        XYpoint_Arr[3][0], XYpoint_Arr[3][1],
                        Dsqrt * 100);
            Driver_Delay_ms(1000);
            TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                         sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
            TP_DrawCross(Mar_Val, Mar_Val, RED);
            continue;
          }//

          //3.Compare diagonal
          Dx = abs((int16_t)(XYpoint_Arr[1][0] -
                             XYpoint_Arr[2][0]));//x1 - x3
          Dy = abs((int16_t)(XYpoint_Arr[1][1] -
                             XYpoint_Arr[2][1]));//y1 - y3
          Dx *= Dx;
          Dy *= Dy;
          Sqrt1 = sqrt(Dx + Dy);//;

          Dx = abs((int16_t)(XYpoint_Arr[0][0] -
                             XYpoint_Arr[3][0]));//x2 - x4
          Dy = abs((int16_t)(XYpoint_Arr[0][1] -
                             XYpoint_Arr[3][1]));//y2 - y4
          Dx *= Dx;
          Dy *= Dy;
          Sqrt2 = sqrt(Dx + Dy);//

          Dsqrt = (float)Sqrt1 / Sqrt2;
          if (Dsqrt < 0.95 || Dsqrt > 1.05) {
            DEBUG("Adjust diagonal direction\r\n");
            cnt = 0;
            TP_ShowInfo(XYpoint_Arr[0][0], XYpoint_Arr[0][1],
                        XYpoint_Arr[1][0], XYpoint_Arr[1][1],
                        XYpoint_Arr[2][0], XYpoint_Arr[2][1],
                        XYpoint_Arr[3][0], XYpoint_Arr[3][1],
                        Dsqrt * 100);
            Driver_Delay_ms(1000);
            TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                         sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
            TP_DrawCross(Mar_Val, Mar_Val, RED);
            continue;
          }

          //4.Get the scale factor and offset
          //Get the scanning direction of the touch screen
          sTP_DEV.TP_Scan_Dir = sLCD_DIS.LCD_Scan_Dir;
          sTP_DEV.fXfac = 0;

          //According to the display direction to get
          //the corresponding scale factor and offset
          if (sTP_DEV.TP_Scan_Dir == R2L_D2U) {
            DEBUG("R2L_D2U\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[1][0] -
                                      XYpoint_Arr[0][0]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[2][1] -
                                      XYpoint_Arr[0][1]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[1][0] +
                                              XYpoint_Arr[0][0])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page -
                             sTP_DEV.fYfac * (XYpoint_Arr[2][1] +
                                              XYpoint_Arr[0][1])
                            ) / 2;

          } else if (sTP_DEV.TP_Scan_Dir == L2R_U2D) {
            DEBUG("L2R_U2D\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][0] -
                                      XYpoint_Arr[1][0]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][1] -
                                      XYpoint_Arr[2][1]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[0][0] +
                                              XYpoint_Arr[1][0])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page - sTP_DEV.fYfac *
                             (XYpoint_Arr[0][1] + XYpoint_Arr[2][1])) / 2;
          } else if (sTP_DEV.TP_Scan_Dir == U2D_R2L) {
            DEBUG("U2D_R2L\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[1][1] - XYpoint_Arr[0][1]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[2][0] - XYpoint_Arr[0][0]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[1][1] +
                                              XYpoint_Arr[0][1])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page -
                             sTP_DEV.fYfac * (XYpoint_Arr[2][0] +
                                              XYpoint_Arr[0][0])
                            ) / 2;
          } else {
            DEBUG("D2U_L2R\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][1] -
                                      XYpoint_Arr[1][1]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][0] -
                                      XYpoint_Arr[2][0]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[0][1] +
                                              XYpoint_Arr[1][1])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page -
                             sTP_DEV.fYfac * (XYpoint_Arr[0][0] +
                                              XYpoint_Arr[2][0])
                            ) / 2;
          }

          DEBUG("sTP_DEV.fXfac = %f \r\n");
          DEBUG(sTP_DEV.fXfac);
          DEBUG("sTP_DEV.fYfac = %f \r\n");
          DEBUG(sTP_DEV.fYfac);
          DEBUG("sTP_DEV.iXoff = %d \r\n");
          DEBUG(sTP_DEV.iXoff);
          DEBUG("sTP_DEV.iYoff = %d \r\n");
          DEBUG( sTP_DEV.iYoff);

          //6.Calibration is successful
          DEBUG("Adjust OK\r\n");
          LCD_Clear(LCD_BACKGROUND);
          GUI_DisString_EN(35, 110, "Touch Screen Adjust OK!",
                           &Font16 , FONT_BACKGROUND , RED);
          Driver_Delay_ms(1000);
          LCD_Clear(LCD_BACKGROUND);
          return;
        //Exception handling,Reset  Initial value
        default :
          cnt = 0;
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                       sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
          TP_DrawCross(Mar_Val, Mar_Val, RED);
          GUI_DisString_EN(40, 26, "TP Need readjust!",
                           &Font16 , FONT_BACKGROUND , RED);
          break;
      }
    }
  }
}
