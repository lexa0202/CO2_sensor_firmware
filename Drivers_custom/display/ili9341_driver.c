#include <ili9341_driver.h>
#include "main.h"


#define LCD_DATA_PORT GPIOE
#define LCD_DATA_MASK 0x00FF   // PE0–PE7

static inline void LCD_Select(void);
static inline void LCD_Unselect(void);

void LCD_HardwareReset(void)
{
    DISP_RES_GPIO_Port->BSRR = (uint32_t)DISP_RES_Pin << 16;
    HAL_Delay(20);
    DISP_RES_GPIO_Port->BSRR = DISP_RES_Pin;
    HAL_Delay(150);
}
static inline void LCD_SetData(uint8_t data)
{
    uint32_t clearMask = ((uint32_t)LCD_DATA_MASK) << 16;
    LCD_DATA_PORT->BSRR = clearMask;        // очистить D0-D7
    LCD_DATA_PORT->BSRR = data;             // установить нужные
}


static inline void LCD_WriteStrobe(void)
{
	DISP_WRX_GPIO_Port->BSRR = (uint32_t)DISP_WRX_Pin << 16; // WR = 0
	DISP_WRX_GPIO_Port->BSRR = DISP_WRX_Pin;                 // WR = 1
}


void LCD_WriteCommand(uint8_t cmd)
{
    DISP_SCL_GPIO_Port->BSRR = (uint32_t)DISP_SCL_Pin << 16; // D/C=0
    LCD_SetData(cmd);
    LCD_WriteStrobe();

}

void LCD_WriteData(uint8_t data)
{
	//!IMPORTANT!
	//CS selected before
    DISP_SCL_GPIO_Port->BSRR = DISP_SCL_Pin; // D/C=1
    LCD_SetData(data);
    LCD_WriteStrobe();


}

void ILI9341_Init(void)
{
    // IMPORTANT!
	//Reset done before
	LCD_Select();
    LCD_WriteCommand(0x01); // Software Reset
    HAL_Delay(100);

    LCD_WriteCommand(0x28); // Display OFF

    // Power control A
    LCD_WriteCommand(0xCB);
    LCD_WriteData(0x39);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x34);
    LCD_WriteData(0x02);

    // Power control B
    LCD_WriteCommand(0xCF);
    LCD_WriteData(0x00);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x30);

    // Driver timing control A
    LCD_WriteCommand(0xE8);
    LCD_WriteData(0x85);
    LCD_WriteData(0x00);
    LCD_WriteData(0x78);

    // Power on sequence control
    LCD_WriteCommand(0xED);
    LCD_WriteData(0x64);
    LCD_WriteData(0x03);
    LCD_WriteData(0x12);
    LCD_WriteData(0x81);

    // Pump ratio control
    LCD_WriteCommand(0xF7);
    LCD_WriteData(0x20);

    // Power control
    LCD_WriteCommand(0xC0);
    LCD_WriteData(0x23);

    LCD_WriteCommand(0xC1);
    LCD_WriteData(0x10);

    // VCOM control
    LCD_WriteCommand(0xC5);
    LCD_WriteData(0x3E);
    LCD_WriteData(0x28);

    LCD_WriteCommand(0xC7);
    LCD_WriteData(0x86);

    // Memory access control
    LCD_WriteCommand(0x36);
    LCD_WriteData(0x48);

    // Pixel format
    LCD_WriteCommand(0x3A);
    LCD_WriteData(0x55); // 16-bit

    LCD_WriteCommand(0xB1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x18);

    LCD_WriteCommand(0xB6);
    LCD_WriteData(0x08);
    LCD_WriteData(0x82);
    LCD_WriteData(0x27);

    LCD_WriteCommand(0x11); // Sleep Out
    HAL_Delay(120);

    LCD_WriteCommand(0x29); // Display ON
    HAL_Delay(20);

    LCD_Unselect();
}

void ILI9341_Fill(uint16_t color)
{
    uint32_t i;
    uint8_t high = color >> 8;
    uint8_t low  = color & 0xFF;

    LCD_Select();
    // Column address set (0..239)
    LCD_WriteCommand(0x2A);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0xEF);   // 239

    // Page address set (0..319)
    LCD_WriteCommand(0x2B);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x01);
    LCD_WriteData(0x3F);   // 319

    // Memory write
    LCD_WriteCommand(0x2C);

    // fill all screen
    DISP_SCL_GPIO_Port->BSRR = DISP_SCL_Pin; // D/C=1

    for (i = 0; i < 240UL * 320UL; i++)
    {
        LCD_SetData(high);
        LCD_WriteStrobe();

        LCD_SetData(low);
        LCD_WriteStrobe();
    }

    LCD_Unselect();

}

static inline void LCD_Select(void)  		// CS = 0
{
    DISP_CS_GPIO_Port->BSRR = (uint32_t)DISP_CS_Pin << 16;
}

static inline void LCD_Unselect(void) 		// CS =1
{
    DISP_CS_GPIO_Port->BSRR = DISP_CS_Pin;
}

void ILI9341_BeginFrame(void)
{
    LCD_Select();

    LCD_WriteCommand(0x2A);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0xEF);

    LCD_WriteCommand(0x2B);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x01);
    LCD_WriteData(0x3F);

    LCD_WriteCommand(0x2C);

    DISP_SCL_GPIO_Port->BSRR = DISP_SCL_Pin;
}

void ILI9341_PushData(const uint8_t* data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        LCD_SetData(data[i]);
        LCD_WriteStrobe();
    }
}

void ILI9341_EndFrame(void)
{
    LCD_Unselect();
}

void ILI9341_WriteLine(uint16_t y, const uint16_t* data)
{
    LCD_Select();

    /* Column address (0..239) */
    LCD_WriteCommand(0x2A);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0xEF);

    /* Page address (y..y) */
    LCD_WriteCommand(0x2B);
    LCD_WriteData((y >> 8) & 0xFF);
    LCD_WriteData(y & 0xFF);
    LCD_WriteData((y >> 8) & 0xFF);
    LCD_WriteData(y & 0xFF);

    /* Memory write */
    LCD_WriteCommand(0x2C);
    DISP_SCL_GPIO_Port->BSRR = DISP_SCL_Pin;

    for (uint16_t x = 0; x < 240; x++)
    {
        uint16_t pixel = data[x];
        LCD_SetData(pixel >> 8);
        LCD_WriteStrobe();
        LCD_SetData(pixel & 0xFF);
        LCD_WriteStrobe();
    }

    LCD_Unselect();
}

void ILI9341_DrawImage(uint16_t x,
                       uint16_t y,
                       uint16_t w,
                       uint16_t h,
                       const uint8_t *data)
{
    LCD_Select();

    LCD_WriteCommand(0x2A);
    LCD_WriteData(x >> 8);
    LCD_WriteData(x & 0xFF);
    LCD_WriteData((x + w - 1) >> 8);
    LCD_WriteData((x + w - 1) & 0xFF);

    LCD_WriteCommand(0x2B);
    LCD_WriteData(y >> 8);
    LCD_WriteData(y & 0xFF);
    LCD_WriteData((y + h - 1) >> 8);
    LCD_WriteData((y + h - 1) & 0xFF);

    LCD_WriteCommand(0x2C);

    DISP_SCL_GPIO_Port->BSRR = DISP_SCL_Pin;

    for (uint32_t i = 0; i < w * h * 2; i++)
    {
        LCD_SetData(data[i]);
        LCD_WriteStrobe();
    }

    LCD_Unselect();
}
