#ifndef LCD_H
#define LCD_H

#include <stdint.h>

static void LCD_SetData(uint8_t data);
static void LCD_WriteStrobe(void);
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);
static void LCD_SetDataInput(void);
static void LCD_SetDataOutput(void);
static uint8_t LCD_ReadByte(void);
void LCD_ReadID(void);
void ILI9341_Init(void);
void ILI9341_FillRed(void);
void ILI9341_Fill(uint16_t color);
#endif
