/******************************************************************************
 * bitmap_renderer.c
 *
 * Line-based monochrome bitmap renderer.
 *
 * Used for dashboard icons, symbols and other 1-bit graphics.
 *
 ******************************************************************************/

#include "bitmap_renderer.h"

void Bitmap_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,

    int16_t bitmapX,
    int16_t bitmapY,

    const Bitmap_t* bitmap,

    uint16_t color
)
{
    if(y < bitmapY)
    {
        return;
    }

    if(y >= bitmapY + bitmap->height)
    {
        return;
    }

    uint16_t localY =
        y - bitmapY;

    uint16_t bytesPerRow =
        (bitmap->width + 7) / 8;

    const uint8_t* row =
		bitmap->bitmap +
        localY * bytesPerRow;

    for(uint16_t x = 0;
        x < bitmap->width;
        x++)
    {
        uint8_t byte =
            row[x / 8];

        uint8_t bit =
            7 - (x % 8);

        if(byte & (1 << bit))
        {
            int16_t screenX =
                bitmapX + x;

            if(screenX >= 0 &&
               screenX < lineWidth)
            {
                lineBuffer[screenX] =
                    color;
            }
        }
    }
}
