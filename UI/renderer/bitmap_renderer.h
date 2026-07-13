#ifndef BITMAP_RENDERER_H
#define BITMAP_RENDERER_H

/******************************************************************************
 * bitmap_renderer.h
 *
 * Monochrome bitmap renderer for scanline-based UI drawing.
 *
 ******************************************************************************/

#include <stdint.h>

typedef struct
{
    uint16_t width;
    uint16_t height;

    const uint8_t* bitmap;

} Bitmap_t;

/*
 * Backward compatibility.
 *
 * TODO:
 * Remove Icon_t after BitmapRenderer migration.
 */
typedef Bitmap_t Icon_t;

void Bitmap_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,

    int16_t bitmapX,
    int16_t bitmapY,

    const Bitmap_t* bitmap,

    uint16_t color
);

#endif /* BITMAP_RENDERER_H */
