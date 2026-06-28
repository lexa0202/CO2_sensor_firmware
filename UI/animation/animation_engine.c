/******************************************************************************
 * animation_engine.c
 *
 * Animation subsystem.
 *
 * Current implementation:
 * - test animation
 * - color cycling
 *
 * Future implementation:
 * - CO2 character emotions
 * - blinking eyes
 * - dashboard effects
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "animation_engine.h"

#include <ili9341_driver.h>

/******************************************************************************
 * Configuration
 *****************************************************************************/

/*
 * Animation update period.
 */
#define COLOR_CHANGE_PERIOD_MS    1000U

/*
 * Test animation color step.
 */
#define COLOR_STEP                0x1111

/******************************************************************************
 * Static data
 *****************************************************************************/

static uint32_t animationTimer =
    0;

static uint16_t animationColor =
    0x0000;

/******************************************************************************
 * Public functions
 *****************************************************************************/

void Animation_Init(void)
{
    animationTimer =
        0;

    animationColor =
        0x0000;
}

void Animation_Update(
    uint32_t dt_ms
)
{
    animationTimer +=
        dt_ms;

    if(animationTimer >=
       COLOR_CHANGE_PERIOD_MS)
    {
        animationTimer = 0;

        animationColor +=
            COLOR_STEP;
    }
}

void Animation_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer
)
{
    (void)y;

    for(uint16_t x = 0;
        x < LCD_WIDTH;
        x++)
    {
        lineBuffer[x] =
            animationColor;
    }
}
