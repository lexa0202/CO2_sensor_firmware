#ifndef GFX_TEXT_RENDERER_H
#define GFX_TEXT_RENDERER_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

#include "gfxfont.h"

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Draw a single text line into a display line buffer.
 *
 * Used by line-based screen rendering.
 */
void GFX_DrawStringLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    const char* text,
    int16_t startX,
    uint16_t textColor,
    const GFXfont* font
);

/*
 * Calculate rendered string width.
 */
uint16_t GFX_GetStringWidth(
    const char* text,
    const GFXfont* font
);

#endif /* GFX_TEXT_RENDERER_H */
