#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <stdint.h>

void Text_DrawStringLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    const char* text,
    int16_t startX,
    uint16_t textColor
);

uint16_t Text_GetStringWidth(const char* text);

#endif
