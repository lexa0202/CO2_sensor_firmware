#include "lcd.h"
#include "main.h"


#define LCD_DATA_PORT GPIOE
#define LCD_DATA_MASK 0x00FF   // PE0–PE7

extern UART_HandleTypeDef huart6;

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

static void LCD_SetDataInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DISP_D0_Pin|DISP_D1_Pin|DISP_D2_Pin|DISP_D3_Pin|
                          DISP_D4_Pin|DISP_D5_Pin|DISP_D6_Pin|DISP_D7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

static void LCD_SetDataOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DISP_D0_Pin|DISP_D1_Pin|DISP_D2_Pin|DISP_D3_Pin|
                          DISP_D4_Pin|DISP_D5_Pin|DISP_D6_Pin|DISP_D7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

static uint8_t LCD_ReadByte(void)
{
    uint8_t value = 0;

    HAL_GPIO_WritePin(DISP_RD_GPIO_Port, DISP_RD_Pin, GPIO_PIN_RESET);
    __NOP(); __NOP(); __NOP();

    if (HAL_GPIO_ReadPin(DISP_D0_GPIO_Port, DISP_D0_Pin)) value |= 0x01;
    if (HAL_GPIO_ReadPin(DISP_D1_GPIO_Port, DISP_D1_Pin)) value |= 0x02;
    if (HAL_GPIO_ReadPin(DISP_D2_GPIO_Port, DISP_D2_Pin)) value |= 0x04;
    if (HAL_GPIO_ReadPin(DISP_D3_GPIO_Port, DISP_D3_Pin)) value |= 0x08;
    if (HAL_GPIO_ReadPin(DISP_D4_GPIO_Port, DISP_D4_Pin)) value |= 0x10;
    if (HAL_GPIO_ReadPin(DISP_D5_GPIO_Port, DISP_D5_Pin)) value |= 0x20;
    if (HAL_GPIO_ReadPin(DISP_D6_GPIO_Port, DISP_D6_Pin)) value |= 0x40;
    if (HAL_GPIO_ReadPin(DISP_D7_GPIO_Port, DISP_D7_Pin)) value |= 0x80;

    HAL_GPIO_WritePin(DISP_RD_GPIO_Port, DISP_RD_Pin, GPIO_PIN_SET);

    return value;
}

void LCD_ReadID(void)
{
    uint8_t buffer[100];
    uint16_t len;

    uint8_t dummy, id1, id2, id3;

    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_RESET);

    // Command
    HAL_GPIO_WritePin(DISP_SCL_GPIO_Port, DISP_SCL_Pin, GPIO_PIN_RESET);
    LCD_SetDataOutput();
    LCD_SetData(0xD3);
    LCD_WriteStrobe();

    // Data
    HAL_GPIO_WritePin(DISP_SCL_GPIO_Port, DISP_SCL_Pin, GPIO_PIN_SET);
    LCD_SetDataInput();

    dummy = LCD_ReadByte();
    id1 = LCD_ReadByte();
    id2 = LCD_ReadByte();
    id3 = LCD_ReadByte();

    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_SET);

    LCD_SetDataOutput();

    len = snprintf((char*)buffer, sizeof(buffer),
                   "LCD ID: %02X %02X %02X %02X\r\n",
                   dummy, id1, id2, id3);

    HAL_UART_Transmit(&huart6, buffer, len, HAL_MAX_DELAY);
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

void ILI9341_FillRed(void)
{
    uint32_t i;

    LCD_WriteCommand(0x2A); // Column address
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0xEF); // 239

    LCD_WriteCommand(0x2B); // Page address
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x01);
    LCD_WriteData(0x3F); // 319

    LCD_WriteCommand(0x2C); // Memory write

    for (i = 0; i < 240*320; i++)
    {
        LCD_WriteData(0xF8); // Red high byte
        LCD_WriteData(0x00); // Red low byte
    }
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
