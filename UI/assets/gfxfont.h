#ifndef GFXFONT_H
#define GFXFONT_H

/******************************************************************************
 * gfxfont.h
 *
 * Minimal Adafruit GFX font structures.
 *
 * Used by:
 * - gfx_text_renderer
 * - dashboard widgets
 * - screen rendering subsystem
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Types
 *****************************************************************************/

/*
 * Single glyph description.
 */
typedef struct
{
    /*
     * Offset into font bitmap table.
     */
    uint16_t bitmapOffset;

    /*
     * Glyph size in pixels.
     */
    uint8_t width;
    uint8_t height;

    /*
     * Cursor advance after drawing glyph.
     */
    uint8_t xAdvance;

    /*
     * Glyph offset relative to cursor position.
     */
    int8_t xOffset;
    int8_t yOffset;

} GFXglyph;

/*
 * Font description.
 */
typedef struct
{
    uint16_t first;
    uint16_t last;

    const GFXglyph* glyph;

} FontRange_t;


typedef struct
{
    /*
     * Bitmap data.
     */
    const uint8_t* bitmap;

    /*
     * Primary glyph table (ASCII).
     */
    const GFXglyph* glyph;

    /*
     * Primary character range.
     */
    uint8_t first;
    uint8_t last;

    /*
     * Line spacing.
     */
    uint8_t yAdvance;

    /*
     * Optional additional Unicode ranges.
     */
    const FontRange_t* ranges;

    uint8_t rangeCount;

} GFXfont;


#endif /* GFXFONT_H */
