#ifndef ICON_RENDERER_H
#define ICON_RENDERER_H

/******************************************************************************
 * icon_renderer.h
 *
 * Monochrome bitmap renderer.
 *
 * Icons are stored as 1-bit-per-pixel bitmaps in Flash.
 * Rendering is performed line-by-line to match the display pipeline.
 *
 ******************************************************************************/

#include <stdint.h>

typedef struct
{
    uint16_t width;
    uint16_t height;

    const uint8_t* bitmap;

} Icon_t;

void Icon_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,

    int16_t iconX,
    int16_t iconY,

    const Icon_t* icon,

    uint16_t color
);

#endif /* ICON_RENDERER_H */
