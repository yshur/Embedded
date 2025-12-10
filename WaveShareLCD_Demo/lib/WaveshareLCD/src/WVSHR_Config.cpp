/******************************************************************************
**************************Hardware interface layer*****************************
* | file          :   WVSHR_Config.cpp
* | version     :   V1.0
* | date        :   2017-12-11
* | function    :
    Provide the hardware underlying interface
******************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include "WVSHR_Config.h"

/********************************************************************************
  function:    System Init and exit
  note:
    Initialize the communication method
    Call before any other calls to LCD or Touch-Pad
********************************************************************************/
uint8_t Wvshr_Init(void)
{
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  
  pinMode(TP_CS, OUTPUT);
  pinMode(TP_IRQ, INPUT);
  pinMode(TP_BUSY, INPUT);

  digitalWrite(TP_IRQ, HIGH);
  digitalWrite(TP_CS, HIGH);
  digitalWrite(LCD_CS, HIGH);

  if (LCD_BL) {
    ledcSetup(0, 5000.0, 8);
    ledcAttachPin(LCD_BL, 0);
  }

  SPI.begin();                      //Default to VSPI: SCK:18, MISO:19, MOSI:23
  SPI.beginTransaction(SPISettings(8 * 1000 * 1000, MSBFIRST, SPI_MODE0));

  return 0;
}

void PWM_SetValue(uint16_t value)
{
  if (value>255)
    value=255;
  if (LCD_BL)
    ledcWrite(0, value);
}

/********************************************************************************
  function:    Delay function
  note:
    Driver_Delay_ms(xms) : Delay x ms
    Driver_Delay_us(xus) : Delay x us
********************************************************************************/
void Driver_Delay_ms(unsigned long xms)
{
  delay(xms);
}

void Driver_Delay_us(int xus)
{
  delayMicroseconds(xus);
}
