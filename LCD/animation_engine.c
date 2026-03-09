#include "animation_engine.h"
#include "lcd.h"

static uint32_t animationTimer = 0;
static uint16_t color = 0x0000;

void Animation_Init(void)
{
    animationTimer = 0;
    color = 0x0000;
}

void Animation_Update(uint32_t dt_ms)
{
    animationTimer += dt_ms;

    if (animationTimer >= 1000)
    {
        animationTimer = 0;
        color += 0x1111;  // простая смена цвета
    }
}

void Animation_RenderLine(uint16_t y, uint16_t* lineBuffer)
{
    for (uint16_t x = 0; x < 240; x++)
    {
        lineBuffer[x] = color;
    }
}
