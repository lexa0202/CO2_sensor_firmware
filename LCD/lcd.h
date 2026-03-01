#ifndef LCD_H
#define LCD_H

#include <stdint.h>
/*
 * D/C = 0 - command mode for LCD
 * D/C = 1 - data mode for LCD
 * CS = 0 - LCD selected
 * CS = 1 - LCD unselected
 * WR = 0 - short strobe, LCD check D0-D7, write byte inside
 * WR = 1 - LCD doesn't receive anything
 * RD = 0 - LCD hold D0-D7 for reading from STM32
 * RD = 1 - LCD do nothing
 */

void LCD_HardwareReset(void);
static inline void LCD_SetData(uint8_t data);
static void LCD_WriteStrobe(void);
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data);
static void LCD_SetDataInput(void);
static void LCD_SetDataOutput(void);
static uint8_t LCD_ReadByte(void);
void LCD_ReadID(void);
static inline void LCD_Select(void);
static inline void LCD_Unselect(void);
void ILI9341_Init(void);
void ILI9341_FillRed(void);
void ILI9341_Fill(uint16_t color);
void ILI9341_DrawRawFromSD(const char* filename);

#endif
