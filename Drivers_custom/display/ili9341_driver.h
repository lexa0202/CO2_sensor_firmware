#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_WIDTH   240
#define LCD_HEIGHT  320
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
void ILI9341_Init(void);
void ILI9341_WriteLine(uint16_t y, const uint16_t* data);
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data);
void ILI9341_Fill(uint16_t color);
void ILI9341_BeginFrame(void);
void ILI9341_PushData(const uint8_t* data, uint32_t length);
void ILI9341_EndFrame(void);



#endif
