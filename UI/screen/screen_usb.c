#include "screen_usb.h"

#include "gfx_text_renderer.h"
#include "FreeSans12pt7b.h"
#include "usb_icon.h"
#include "ili9341_driver.h"

#define USB_TEXT_COLOR   0xFFFF
#define USB_BG_COLOR     0x0000
#define USB_LINE_SPACING 8

#define USB_LINE_HEIGHT      32

#define USB_LINE1_X          20
#define USB_LINE2_X          80

#define USB_TEXT_START_Y     70

#define USB_ICON_X           ((LCD_WIDTH - USB_ICON_W) / 2)
#define USB_ICON_Y           140

static uint16_t lineBuffer[LCD_WIDTH];

void ScreenUsb_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
)
{
    static const char* line1 =
        "USB is connected";

    static const char* line2 =
        "to PC";

    for(uint16_t i = 0; i < width; i++)
    {
        lineBuffer[i] = USB_BG_COLOR;
    }

    if(y >= USB_TEXT_START_Y &&
       y < USB_TEXT_START_Y + USB_LINE_HEIGHT)
    {
        GFX_DrawStringLine(
            y - USB_TEXT_START_Y,
            lineBuffer,
            width,
            line1,
            USB_LINE1_X,
            USB_TEXT_COLOR,
            &FreeSans12pt7b
        );
    }

    if(y >= USB_TEXT_START_Y +
            USB_LINE_HEIGHT +
            USB_LINE_SPACING &&
       y < USB_TEXT_START_Y +
            USB_LINE_HEIGHT * 2 +
            USB_LINE_SPACING)
    {
        GFX_DrawStringLine(
            y - (
                USB_TEXT_START_Y +
                USB_LINE_HEIGHT +
                USB_LINE_SPACING
            ),
            lineBuffer,
            width,
            line2,
            USB_LINE2_X,
            USB_TEXT_COLOR,
            &FreeSans12pt7b
        );
    }
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

    ILI9341_DrawImage(
        USB_ICON_X,
        USB_ICON_Y,
        USB_ICON_W,
        USB_ICON_H,
        usb_icon
    );
}
