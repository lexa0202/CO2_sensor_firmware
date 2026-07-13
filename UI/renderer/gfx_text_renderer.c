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
#include <stddef.h>

/******************************************************************************
 * Configuration
 *****************************************************************************/

static int16_t GFX_GetBaselineY(
    const GFXfont* font
)
{
    return (int16_t)font->yAdvance + 1;
}

int16_t GFX_GetFontBaselineY(
    const GFXfont* font
)
{
    return GFX_GetBaselineY(font);
}

static uint32_t GFX_NextCodepoint(
    const char** text
)
{
    const uint8_t* s =
        (const uint8_t*)(*text);

    uint32_t codepoint;

    /*
     * ASCII
     */
    if(s[0] < 0x80)
    {
        *text += 1;
        return s[0];
    }

    /*
     * 2-byte UTF-8
     */
    if((s[0] & 0xE0) == 0xC0)
    {
        if((s[1] & 0xC0) != 0x80)
        {
            (*text)++;
            return '?';
        }

        codepoint =
            ((uint32_t)(s[0] & 0x1F) << 6) |
            ((uint32_t)(s[1] & 0x3F));

        *text += 2;

        return codepoint;
    }

    /*
     * 3-byte UTF-8
     */
    if((s[0] & 0xF0) == 0xE0)
    {
        if(((s[1] & 0xC0) != 0x80) ||
           ((s[2] & 0xC0) != 0x80))
        {
            (*text)++;
            return '?';
        }

        codepoint =
            ((uint32_t)(s[0] & 0x0F) << 12) |
            ((uint32_t)(s[1] & 0x3F) << 6) |
            ((uint32_t)(s[2] & 0x3F));

        *text += 3;

        return codepoint;
    }

    /*
     * 4-byte UTF-8
     */
    if((s[0] & 0xF8) == 0xF0)
    {
        if(((s[1] & 0xC0) != 0x80) ||
           ((s[2] & 0xC0) != 0x80) ||
           ((s[3] & 0xC0) != 0x80))
        {
            (*text)++;
            return '?';
        }

        codepoint =
            ((uint32_t)(s[0] & 0x07) << 18) |
            ((uint32_t)(s[1] & 0x3F) << 12) |
            ((uint32_t)(s[2] & 0x3F) << 6) |
            ((uint32_t)(s[3] & 0x3F));

        *text += 4;

        return codepoint;
    }

    /*
     * Invalid UTF-8 start byte.
     */
    (*text)++;

    return '?';
}

static const GFXglyph* GFX_FindGlyph(
    const GFXfont* font,
    uint32_t codepoint
)
{
    /*
     * Primary range.
     */
    if(codepoint >= font->first &&
       codepoint <= font->last)
    {
        return &font->glyph[
            codepoint - font->first
        ];
    }

    /*
     * Additional Unicode ranges.
     */
    if(font->ranges != NULL)
    {
        for(uint8_t i = 0;
            i < font->rangeCount;
            i++)
        {
            const FontRange_t* range =
                &font->ranges[i];

            if(codepoint >= range->first &&
               codepoint <= range->last)
            {
                return &range->glyph[
                    codepoint -
                    range->first
                ];
            }
        }
    }

    return NULL;
}

static void GFX_DrawGlyphLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    int16_t cursorX,
    int16_t baselineY,
    uint16_t textColor,
    const GFXfont* font,
    const GFXglyph* glyph
)
{
    const int16_t glyphTop =
        baselineY +
        glyph->yOffset;

    const int16_t glyphBottom =
        glyphTop +
        glyph->height;

    if(y < glyphTop ||
       y >= glyphBottom)
    {
        return;
    }

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
        uint32_t codepoint =
            GFX_NextCodepoint(&text);

        const GFXglyph* glyph =
            GFX_FindGlyph(
                font,
                codepoint
            );

        if(glyph == NULL)
        {
            continue;
        }

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
    GFX_DrawStringAtBaseline(
        y,
        lineBuffer,
        bufferWidth,
        text,
        startX,
        GFX_GetBaselineY(font),
        textColor,
        font
    );
}

void GFX_DrawStringAtBaseline(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    const char* text,
    int16_t startX,
    int16_t baselineY,
    uint16_t textColor,
    const GFXfont* font
)
{
    int16_t cursorX = startX;

    while(*text)
    {
        uint32_t codepoint =
            GFX_NextCodepoint(&text);

        const GFXglyph* glyph =
            GFX_FindGlyph(
                font,
                codepoint
            );

        if(glyph == NULL)
        {
            continue;
        }

        GFX_DrawGlyphLine(
            y,
            lineBuffer,
            bufferWidth,
            cursorX,
            baselineY,
            textColor,
            font,
            glyph
        );

        cursorX += glyph->xAdvance;
    }
}

uint16_t GFX_GetCo2LabelWidth(
    const GFXfont* mainFont,
    const GFXfont* subFont
)
{
    return
        GFX_GetStringWidth("CO", mainFont) +
        GFX_GetStringWidth("2", subFont);
}

void GFX_DrawCo2LabelLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t bufferWidth,
    int16_t centerX,
    uint16_t textColor,
    const GFXfont* mainFont,
    const GFXfont* subFont
)
{
    const uint16_t totalWidth =
        GFX_GetCo2LabelWidth(
            mainFont,
            subFont
        );

    int16_t cursorX =
        centerX -
        (int16_t)(totalWidth / 2);

    const int16_t mainBaseline = 17;

    const int16_t subBaseline = 25;

    const char* text = "CO";

    while(*text)
    {
        uint32_t codepoint =
            GFX_NextCodepoint(&text);

        const GFXglyph* glyph =
            GFX_FindGlyph(
                mainFont,
                codepoint
            );

        if(glyph != NULL)
        {
            GFX_DrawGlyphLine(
                y,
                lineBuffer,
                bufferWidth,
                cursorX,
                mainBaseline,
                textColor,
                mainFont,
                glyph
            );

            cursorX += glyph->xAdvance;
        }
    }

    text = "2";

    while(*text)
    {
        uint32_t codepoint =
            GFX_NextCodepoint(&text);

        const GFXglyph* glyph =
            GFX_FindGlyph(
                subFont,
                codepoint
            );

        if(glyph != NULL)
        {
            GFX_DrawGlyphLine(
                y,
                lineBuffer,
                bufferWidth,
                cursorX,
                subBaseline,
                textColor,
                subFont,
                glyph
            );

            cursorX += glyph->xAdvance;
        }
    }
}
