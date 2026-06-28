/******************************************************************************
 * gfx_text_renderer.c
 *
 * Lightweight line-based text renderer for Adafruit GFX fonts.
 *
 * Designed for memory-constrained STM32 targets where the display is rendered
 * one scanline at a time.
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "gfx_text_renderer.h"

/******************************************************************************
 * Configuration
 *****************************************************************************/

/*
 * Baseline position used by FreeSans12pt7b.
 *
 * If another font family is introduced in the future,
 * this value may need adjustment.
 */
#define GFX_TEXT_BASELINE_Y     22

/******************************************************************************
 * Public functions
 *****************************************************************************/

uint16_t GFX_GetStringWidth(
    const char* text,
    const GFXfont* font
)
{
    uint16_t width = 0;

    while(*text)
    {
        char c = *text++;

        if(c < font->first ||
           c > font->last)
        {
            continue;
        }

        const GFXglyph* glyph =
            &font->glyph[
                c - font->first
            ];

        width += glyph->xAdvance;
    }

    return width;
}

void GFX_DrawStringLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    const char* text,
    int16_t startX,
    uint16_t textColor,
    const GFXfont* font
)
{
    int16_t cursorX =
        startX;

    while(*text)
    {
        char c = *text++;

        if(c < font->first ||
           c > font->last)
        {
            continue;
        }

        const GFXglyph* glyph =
            &font->glyph[
                c - font->first
            ];

        const int16_t glyphTop =
            GFX_TEXT_BASELINE_Y +
            glyph->yOffset;

        const int16_t glyphBottom =
            glyphTop +
            glyph->height;

        /*
         * Skip glyph if current LCD line
         * is outside glyph bounds.
         */
        if(y >= glyphTop &&
           y < glyphBottom)
        {
            const uint8_t targetRow =
                y - glyphTop;

            uint32_t bitmapOffset =
                glyph->bitmapOffset;

            uint8_t bits = 0;
            uint8_t bitMask = 0;

            for(uint8_t yy = 0;
                yy < glyph->height;
                yy++)
            {
                for(uint8_t xx = 0;
                    xx < glyph->width;
                    xx++)
                {
                    if(bitMask == 0)
                    {
                        bits =
                            font->bitmap[
                                bitmapOffset++
                            ];

                        bitMask = 0x80;
                    }

                    if(yy == targetRow)
                    {
                        if(bits & bitMask)
                        {
                            int16_t px =
                                cursorX +
                                glyph->xOffset +
                                xx;

                            if(px >= 0 &&
                               px < bufferWidth)
                            {
                                lineBuffer[px] =
                                    textColor;
                            }
                        }
                    }

                    bitMask >>= 1;
                }
            }
        }

        cursorX +=
            glyph->xAdvance;
    }
}
