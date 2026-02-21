#include "lcd.h"
#include "main.h"

extern UART_HandleTypeDef huart6;

static void LCD_SetData(uint8_t data)
{
    HAL_GPIO_WritePin(DISP_D0_GPIO_Port, DISP_D0_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D1_GPIO_Port, DISP_D1_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D2_GPIO_Port, DISP_D2_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D3_GPIO_Port, DISP_D3_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D4_GPIO_Port, DISP_D4_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D5_GPIO_Port, DISP_D5_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D6_GPIO_Port, DISP_D6_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_D7_GPIO_Port, DISP_D7_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


static void LCD_WriteStrobe(void)
{
    HAL_GPIO_WritePin(DISP_WRX_GPIO_Port, DISP_WRX_Pin, GPIO_PIN_RESET);
    __NOP(); __NOP();
    HAL_GPIO_WritePin(DISP_WRX_GPIO_Port, DISP_WRX_Pin, GPIO_PIN_SET);
}


void LCD_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_SCL_GPIO_Port, DISP_SCL_Pin, GPIO_PIN_RESET); // D/C = 0 (command)

    LCD_SetData(cmd);
    LCD_WriteStrobe();

    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_SET);
}

void LCD_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_SCL_GPIO_Port, DISP_SCL_Pin, GPIO_PIN_SET); // D/C = 1 (data)

    LCD_SetData(data);
    LCD_WriteStrobe();

    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_SET);
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

    // Data phase
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
    // Reset уже сделан

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

    // Заливка всего экрана
    for (i = 0; i < 240UL * 320UL; i++)
    {
        LCD_WriteData(high);
        LCD_WriteData(low);
    }
}

