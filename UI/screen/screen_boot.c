/******************************************************************************
 * screen_boot.c
 *
 * Boot splash screen.
 *
 * Sequence:
 * 1. Black background with white logo and "CO2 EVO" title
 * 2. Hold for 5 seconds
 * 3. Smooth fade-out
 * 4. Finish and switch to dashboard
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "screen_boot.h"

#include "logo.h"
#include "bitmap_renderer.h"
#include "gfx_text_renderer.h"

#include "FreeSans7pt8b.h"
#include "YandexSansDisplay_Bold20pt7b.h"

#include <ili9341_driver.h>
#include "main.h"

/******************************************************************************
 * Configuration
 *****************************************************************************/

#define BOOT_BG_COLOR              0x0000
#define BOOT_FG_COLOR              0xFFFF

#define BOOT_SPLASH_DURATION_MS    5000U
#define BOOT_FADE_DURATION_MS       600U

#define BOOT_LOGO_Y                  48
#define BOOT_TITLE_GAP               18
#define BOOT_TITLE_BAND_HEIGHT       32

static const char BOOT_TITLE_CO[] = "CO";
static const char BOOT_TITLE_SUB[] = "2";
static const char BOOT_TITLE_EVO[] = " EVO";

/******************************************************************************
 * Types
 *****************************************************************************/

typedef enum
{
    BOOT_PHASE_SPLASH,
    BOOT_PHASE_FADE,
    BOOT_PHASE_DONE

} BootPhase_t;

/******************************************************************************
 * Static data
 *****************************************************************************/

static BootPhase_t bootPhase =
    BOOT_PHASE_SPLASH;

static uint32_t phaseStartTick =
    0;

static uint8_t fadeAmount =
    0;

/******************************************************************************
 * Private functions
 *****************************************************************************/

static int16_t Boot_GetLogoX(void)
{
    return (int16_t)(
        (LCD_WIDTH - BitmapLogo150.width) /
        2
    );
}

static int16_t Boot_GetTitleY(void)
{
    return (int16_t)(
        BOOT_LOGO_Y +
        BitmapLogo150.height +
        BOOT_TITLE_GAP
    );
}

static uint16_t Boot_LerpColor(
    uint16_t colorA,
    uint16_t colorB,
    uint8_t amount
)
{
    const uint8_t rA = (colorA >> 11) & 0x1F;
    const uint8_t gA = (colorA >> 5) & 0x3F;
    const uint8_t bA = colorA & 0x1F;

    const uint8_t rB = (colorB >> 11) & 0x1F;
    const uint8_t gB = (colorB >> 5) & 0x3F;
    const uint8_t bB = colorB & 0x1F;

    const uint8_t r =
        rA + (uint8_t)(((int16_t)rB - rA) * amount / 255);

    const uint8_t g =
        gA + (uint8_t)(((int16_t)gB - gA) * amount / 255);

    const uint8_t b =
        bA + (uint8_t)(((int16_t)bB - bA) * amount / 255);

    return (uint16_t)((r << 11) | (g << 5) | b);
}

static uint16_t Boot_ApplyFade(
    uint16_t color
)
{
    if(bootPhase != BOOT_PHASE_FADE)
    {
        return color;
    }

    return Boot_LerpColor(
        color,
        BOOT_BG_COLOR,
        fadeAmount
    );
}

static void Boot_DrawTitleLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
)
{
    const int16_t titleY =
        Boot_GetTitleY();

    if(y < titleY ||
       y >= titleY + BOOT_TITLE_BAND_HEIGHT)
    {
        return;
    }

    const GFXfont* titleFont =
        &YandexSansDisplay_Bold20pt7b;

    const GFXfont* subFont =
        &FreeSans7pt8b;

    const uint16_t coWidth =
        GFX_GetStringWidth(
            BOOT_TITLE_CO,
            titleFont
        );

    const uint16_t subWidth =
        GFX_GetStringWidth(
            BOOT_TITLE_SUB,
            subFont
        );

    const uint16_t evoWidth =
        GFX_GetStringWidth(
            BOOT_TITLE_EVO,
            titleFont
        );

    const uint16_t totalWidth =
        coWidth +
        subWidth +
        evoWidth;

    int16_t cursorX =
        (int16_t)((width - totalWidth) / 2);

    const uint16_t localY =
        y - titleY;

    const int16_t mainBaseline = 20;
    const int16_t subBaseline = 27;

    GFX_DrawStringAtBaseline(
        localY,
        lineBuffer,
        width,
        BOOT_TITLE_CO,
        cursorX,
        mainBaseline,
        BOOT_FG_COLOR,
        titleFont
    );

    cursorX += (int16_t)coWidth;

    GFX_DrawStringAtBaseline(
        localY,
        lineBuffer,
        width,
        BOOT_TITLE_SUB,
        cursorX,
        subBaseline,
        BOOT_FG_COLOR,
        subFont
    );

    cursorX += (int16_t)subWidth;

    GFX_DrawStringAtBaseline(
        localY,
        lineBuffer,
        width,
        BOOT_TITLE_EVO,
        cursorX,
        mainBaseline,
        BOOT_FG_COLOR,
        titleFont
    );
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

void ScreenBoot_Reset(void)
{
    bootPhase =
        BOOT_PHASE_SPLASH;

    phaseStartTick =
        HAL_GetTick();

    fadeAmount =
        0;
}

void ScreenBoot_Update(void)
{
    const uint32_t now =
        HAL_GetTick();

    const uint32_t elapsed =
        now -
        phaseStartTick;

    if(bootPhase == BOOT_PHASE_SPLASH)
    {
        if(elapsed >= BOOT_SPLASH_DURATION_MS)
        {
            bootPhase =
                BOOT_PHASE_FADE;

            phaseStartTick =
                now;

            fadeAmount =
                0;
        }

        return;
    }

    if(bootPhase == BOOT_PHASE_FADE)
    {
        fadeAmount =
            (uint8_t)(
                (elapsed * 255UL) /
                BOOT_FADE_DURATION_MS
            );

        if(elapsed >= BOOT_FADE_DURATION_MS)
        {
            fadeAmount = 255;
            bootPhase = BOOT_PHASE_DONE;
        }
    }
}

uint8_t ScreenBoot_IsFinished(void)
{
    return
        (bootPhase ==
         BOOT_PHASE_DONE) ?
        1U :
        0U;
}

void ScreenBoot_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
)
{
    for(uint16_t x = 0; x < width; x++)
    {
        lineBuffer[x] =
            BOOT_BG_COLOR;
    }

    Bitmap_RenderLine(
        y,
        lineBuffer,
        width,
        Boot_GetLogoX(),
        BOOT_LOGO_Y,
        &BitmapLogo150,
        BOOT_FG_COLOR
    );

    Boot_DrawTitleLine(
        y,
        lineBuffer,
        width
    );

    for(uint16_t x = 0; x < width; x++)
    {
        lineBuffer[x] =
            Boot_ApplyFade(
                lineBuffer[x]
            );
    }
}

void ScreenBoot_DrawFullFrame(void)
{
    uint16_t lineBuffer[LCD_WIDTH];

    for(uint16_t y = 0; y < LCD_HEIGHT; y++)
    {
        ScreenBoot_RenderLine(
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
