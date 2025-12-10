#include <Arduino.h>

#include "WaveShareDemo.h"

void setup()
{
  Wvshr_Init();

  Serial.println("3.5inch TFT Touch Shield LCD Show...");  
  Serial.println("LCD Init...");
  LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;  
  LCD_Init( Lcd_ScanDir, 200);

  Serial.println("LCD_Clear...");
  LCD_Clear(LCD_BACKGROUND);

  Serial.println("LCD_Show...");
  GUI_Show();

  TP_Init();    //Init the TouchPad

  //  ======  Draw button to moveon to time display ======
  GUI_DrawRectangle(200, 170, 360, 170+35, YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
  GUI_DisString_EN(205, 180, "Touch for Time", &Font16, YELLOW, BLUE);

  while(1) {
    TP_Scan(0);
    if (sTP_DEV.chStatus & TP_PRESSED) {
      sTP_DEV.chStatus &= ~TP_PRESSED;
      if (sTP_Draw.Xpoint>200 && sTP_Draw.Xpoint<360 && sTP_Draw.Ypoint>170 && sTP_Draw.Ypoint<170+35)
        break;
    }
    delay(100);
  }

  GUI_DrawRectangle(200, 170, 360, 170+35, WHITE, DRAW_FULL, DOT_PIXEL_1X1);


  //  ======  Prepare button to moveon to Touch-Draw demonstration in loop() ======
  GUI_DrawRectangle(360, 170, 444, 170+35, BLUE, DRAW_FULL, DOT_PIXEL_1X1);
  GUI_DisString_EN(362, 176, "Next=>", &Font20, BLUE, WHITE);
}

void loop()
{
  DEV_TIME sDev_time;
  sDev_time.Hour = 16;
  sDev_time.Min = 00;
  sDev_time.Sec = 00;

  for (;;) {
    sDev_time.Sec++;
  //  ======  Demonstrate updating Time Display ======
    GUI_Showtime(200, 170, 327, 170+47, &sDev_time, RED);
    Driver_Delay_ms(1000);//Analog clock 1s
    if (sDev_time.Sec == 60)
      sDev_time.Sec = 0;

    TP_Scan(0);
    if (sTP_DEV.chStatus & TP_PRESSED) {
      sTP_DEV.chStatus &= ~TP_PRESSED;
      if (sTP_Draw.Xpoint>360 && sTP_Draw.Xpoint<444 && sTP_Draw.Ypoint>170 && sTP_Draw.Ypoint<170+35)
        break;
    }
  }

  //  ======  Demonstrate Touch Drawing ======
  TP_Dialog();
  for (;;) {
    TP_DrawBoard();
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/






