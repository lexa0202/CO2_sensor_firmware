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
    /*
     * Bitmap data for all glyphs.
     */
    const uint8_t* bitmap;

    /*
     * Glyph table.
     */
    const GFXglyph* glyph;

    /*
     * First and last supported ASCII characters.
     */
    uint8_t first;
    uint8_t last;

    /*
     * Recommended line spacing.
     */
    uint8_t yAdvance;

} GFXfont;

#endif /* GFXFONT_H */
