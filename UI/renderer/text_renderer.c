#include "text_renderer.h"
#include "font16x24.h"

uint16_t Text_GetStringWidth(const char* text)
{
    uint16_t len = 0;
    while (*text++)
        len++;

    return len * FONT_ADVANCE;
}

void Text_DrawStringLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    const char* text,
    int16_t startX,
    uint16_t textColor)
{
    int16_t row = (int16_t)y - FONT_BASELINE_OFFSET;

    if (row < 0 || row >= FONT_HEIGHT)
        return;

    uint16_t charIndex = 0;

    while (*text)
    {
        char c = *text++;

        if (c < FONT_FIRST_CHAR || c > FONT_LAST_CHAR)
            c = ' ';

        uint16_t fontIndex = c - FONT_FIRST_CHAR;

        uint8_t byteHigh = font16x24[fontIndex][row][0];
        uint8_t byteLow  = font16x24[fontIndex][row][1];

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (byteHigh & (1 << (7 - bit)))
            {
            	int32_t px = startX + charIndex * FONT_ADVANCE + FONT_SIDE_PADDING + bit;

                if (px >= 0 && px < bufferWidth)
                    lineBuffer[px] = textColor;
            }
        }

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (byteLow & (1 << (7 - bit)))
            {
            	int32_t px = startX + charIndex * FONT_ADVANCE + FONT_SIDE_PADDING + 8 + bit;

                if (px >= 0 && px < bufferWidth)
                    lineBuffer[px] = textColor;
            }
        }

        charIndex++;
    }
}
