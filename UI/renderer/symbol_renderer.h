/******************************************************************************
 * symbol_renderer.h
 *
 * Renderer for small monochrome UI symbols.
 *
 * Used for symbols that are not available in the selected font
 * (degree sign, arrows, check marks, etc.).
 ******************************************************************************/

#ifndef SYMBOL_RENDERER_H
#define SYMBOL_RENDERER_H

#include <stdint.h>

/******************************************************************************
 * Symbol bitmap
 ******************************************************************************/

typedef struct
{
    uint8_t width;
    uint8_t height;

    const uint8_t* bitmap;

} Symbol_t;

/******************************************************************************
 * Public symbols
 ******************************************************************************/

extern const Symbol_t SymbolDegree;

/******************************************************************************
 * Public functions
 ******************************************************************************/

/*
 * Render one scanline of a monochrome symbol.
 */
void Symbol_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,

    int16_t symbolX,
    int16_t symbolY,

    const Symbol_t* symbol,

    uint16_t color
);

#endif
