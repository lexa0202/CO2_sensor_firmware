/******************************************************************************
 * icon_renderer.c
 *
 * Line-based monochrome icon renderer.
 ******************************************************************************/

#include "icon_renderer.h"

void Icon_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,

    int16_t iconX,
    int16_t iconY,

    const Icon_t* icon,

    uint16_t color
)
{
    if(y < iconY)
    {
        return;
    }

    if(y >= iconY + icon->height)
    {
        return;
    }

    uint16_t localY =
        y - iconY;

    uint16_t bytesPerRow =
        (icon->width + 7) / 8;

    const uint8_t* row =
		icon->bitmap +
        localY * bytesPerRow;

    for(uint16_t x = 0;
        x < icon->width;
        x++)
    {
        uint8_t byte =
            row[x / 8];

        uint8_t bit =
            7 - (x % 8);

        if(byte & (1 << bit))
        {
            int16_t screenX =
                iconX + x;

            if(screenX >= 0 &&
               screenX < lineWidth)
            {
                lineBuffer[screenX] =
                    color;
            }
        }
    }
}
