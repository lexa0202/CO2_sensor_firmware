/******************************************************************************
 * gfx_text_renderer.h
 *
 * Line-based text renderer for Adafruit GFX fonts.
 *
 ******************************************************************************/

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
 * Draw text using an explicit baseline inside the local line band.
 */
void GFX_DrawStringAtBaseline(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    const char* text,
    int16_t startX,
    int16_t baselineY,
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

/*
 * Baseline position for a font inside a local text band.
 */
int16_t GFX_GetFontBaselineY(
    const GFXfont* font
);

/*
 * Render "CO" with a subscript "2" centered at centerX.
 *
 * Use GFX_CO2_LABEL_BAND_HEIGHT for the surrounding text band.
 */
#define GFX_CO2_LABEL_BAND_HEIGHT   30U

uint16_t GFX_GetCo2LabelWidth(
    const GFXfont* mainFont,
    const GFXfont* subFont
);

void GFX_DrawCo2LabelLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    int16_t centerX,
    uint16_t textColor,
    const GFXfont* mainFont,
    const GFXfont* subFont
);

#endif /* GFX_TEXT_RENDERER_H */
