/******************************************************************************
 * screen_usb.c
 *
 * USB Mass Storage mode screen.
 *
 * Shows a monochrome USB icon and a Russian status message when the device
 * is connected to a PC.
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "screen_usb.h"

#include "usb.h"
#include "bitmap_renderer.h"
#include "gfx_text_renderer.h"

#include "YandexSansDisplay_Bold20pt7b.h"
#include "YandexSansDisplay_Light20pt7b.h"

#include <ili9341_driver.h>

/******************************************************************************
 * Configuration
 *****************************************************************************/

#define USB_BG_COLOR             0x0000
#define USB_TEXT_COLOR           0xFFFF

#define USB_TEXT_BAND_HEIGHT     34
#define USB_TEXT_LINE1_Y         32
#define USB_TEXT_LINE2_Y         62
#define USB_TEXT_LINE3_Y         92

#define USB_ICON_Y              165

static const char USB_TEXT_LINE1[] =
    u8"Устройство";

static const char USB_TEXT_LINE2[] =
    u8"подключено";

static const char USB_TEXT_LINE3_PREFIX[] =
    u8"к ";

static const char USB_TEXT_LINE3_BOLD[] =
    u8"ПК";

/******************************************************************************
 * Static data
 *****************************************************************************/

static uint16_t lineBuffer[LCD_WIDTH];

/******************************************************************************
 * Private functions
 *****************************************************************************/

static int16_t Usb_GetIconX(void)
{
    return (int16_t)(
        (LCD_WIDTH - BitmapUsb150.width) /
        2
    );
}

static void Usb_DrawCenteredTextBand(
    uint16_t screenY,
    int16_t bandTopY,
    uint16_t* buffer,
    uint16_t width,
    const char* text,
    const GFXfont* font
)
{
    if(screenY < bandTopY ||
       screenY >= bandTopY + USB_TEXT_BAND_HEIGHT)
    {
        return;
    }

    const uint16_t textWidth =
        GFX_GetStringWidth(
            text,
            font
        );

    GFX_DrawStringLine(
        screenY - bandTopY,
        buffer,
        width,
        text,
        (width - textWidth) / 2,
        USB_TEXT_COLOR,
        font
    );
}

static void Usb_DrawLine3Band(
    uint16_t screenY,
    uint16_t* buffer,
    uint16_t width
)
{
    if(screenY < USB_TEXT_LINE3_Y ||
       screenY >= USB_TEXT_LINE3_Y + USB_TEXT_BAND_HEIGHT)
    {
        return;
    }

    const GFXfont* lightFont =
        &YandexSansDisplay_Light20pt7b;

    const GFXfont* boldFont =
        &YandexSansDisplay_Bold20pt7b;

    const uint16_t prefixWidth =
        GFX_GetStringWidth(
            USB_TEXT_LINE3_PREFIX,
            lightFont
        );

    const uint16_t boldWidth =
        GFX_GetStringWidth(
            USB_TEXT_LINE3_BOLD,
            boldFont
        );

    const uint16_t totalWidth =
        prefixWidth +
        boldWidth;

    int16_t cursorX =
        (int16_t)((width - totalWidth) / 2);

    const uint16_t localY =
        screenY -
        USB_TEXT_LINE3_Y;

    const int16_t baseline =
        GFX_GetFontBaselineY(lightFont);

    GFX_DrawStringAtBaseline(
        localY,
        buffer,
        width,
        USB_TEXT_LINE3_PREFIX,
        cursorX,
        baseline,
        USB_TEXT_COLOR,
        lightFont
    );

    cursorX += (int16_t)prefixWidth;

    GFX_DrawStringAtBaseline(
        localY,
        buffer,
        width,
        USB_TEXT_LINE3_BOLD,
        cursorX,
        baseline,
        USB_TEXT_COLOR,
        boldFont
    );
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

void ScreenUsb_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
)
{
    for(uint16_t i = 0; i < width; i++)
    {
        lineBuffer[i] =
            USB_BG_COLOR;
    }

    Usb_DrawCenteredTextBand(
        y,
        USB_TEXT_LINE1_Y,
        lineBuffer,
        width,
        USB_TEXT_LINE1,
        &YandexSansDisplay_Light20pt7b
    );

    Usb_DrawCenteredTextBand(
        y,
        USB_TEXT_LINE2_Y,
        lineBuffer,
        width,
        USB_TEXT_LINE2,
        &YandexSansDisplay_Light20pt7b
    );

    Usb_DrawLine3Band(
        y,
        lineBuffer,
        width
    );

    Bitmap_RenderLine(
        y,
        lineBuffer,
        width,
        Usb_GetIconX(),
        USB_ICON_Y,
        &BitmapUsb150,
        USB_TEXT_COLOR
    );
}

void ScreenUsb_Draw(void)
{
    for(uint16_t y = 0;
        y < LCD_HEIGHT;
        y++)
    {
        ScreenUsb_RenderLine(
            y,
            lineBuffer,
            LCD_WIDTH
        );

        ILI9341_WriteLine(
            y,
            lineBuffer
        );
    }
}
