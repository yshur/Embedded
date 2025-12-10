/******************************************************************************
**************************Hardware interface layer*****************************
* | file          :   WVSHR_Config.h
* | version     :   V1.0
* | date        :   2017-12-11
* | function    :
    Provide the hardware underlying interface
******************************************************************************/
#ifndef _WVSHR_CONFIG_H_
#define _WVSHR_CONFIG_H_

#include <SPI.h>
#include <Wire.h>

#define USE_SPI_4W 1
#define USE_IIC    0

//GPIO config
//LCD
#define LCD_CS 15       //LCD SPI Chip-Select
#define LCD_RST 26
#define LCD_DC 17       //LCD Date/CMD select bit
#define LCD_BL 0        //Optional (PWM) pin to control backlight intensity (otherwise, set to 0 and connect to 3V3)

//Touch
#define TP_CS 4         //Touch pad SPI Chip-Select
#define TP_IRQ 16       //Touch pad signals a touch (can be used to trigger an ISR)
#define TP_BUSY 12      //Optional (Input) pin to detect when touch-pad ADC is busy

//SD
#define SD_CS 5

//LCD I/O Manipulation
#define LCD_CS_0        digitalWrite(LCD_CS, LOW)
#define LCD_CS_1        digitalWrite(LCD_CS, HIGH)

#define LCD_RST_0       digitalWrite(LCD_RST, LOW)
#define LCD_RST_1       digitalWrite(LCD_RST, HIGH)

#define LCD_DC_CMD        digitalWrite(LCD_DC, LOW)
#define LCD_DC_DATA        digitalWrite(LCD_DC, HIGH)

//Touch I/O Manipulation
#define TP_CS_0         digitalWrite(TP_CS, LOW)
#define TP_CS_1         digitalWrite(TP_CS, HIGH)

//SD I/O Manipulation
#define SD_CS_0   digitalWrite(LCD_CS, LOW)
#define SD_CS_1    digitalWrite(LCD_CS, HIGH)

#define GET_TP_IRQ      digitalRead(TP_IRQ)

#define GET_TP_BUSY    digitalRead(TP_BUSY)

#define SPI4W_Write_Byte(__DATA) SPI.transfer(__DATA)
#define SPI4W_Read_Byte(__DATA) SPI.transfer(__DATA)

#define SPI4W_Write_Word(__DATA) SPI.write16(__DATA)

/*------------------------------------------------------------------------------------------------------*/
uint8_t Wvshr_Init(void);
void PWM_SetValue(uint16_t value);

void Driver_Delay_ms(unsigned long xms);
void Driver_Delay_us(int xus);

#endif

