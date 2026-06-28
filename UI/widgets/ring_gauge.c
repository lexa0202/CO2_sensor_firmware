/******************************************************************************
 * ring_gauge.c
 *
 * Circular dashboard widget.
 *
 * Features:
 * - rainbow arc rendering
 * - CO2 scale support
 * - animated position marker
 * - centered text rendering
 *
 * Coordinate system:
 *
 * 0°   = top
 * 90°  = right
 * 180° = bottom
 * 270° = left
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <math.h>

#include "ring_gauge.h"

#include "gfx_text_renderer.h"
#include "FreeSans12pt7b.h"

/******************************************************************************
 * Configuration
 *****************************************************************************/

/*
 * Visible dashboard arc.
 *
 * Blue zone  -> left side
 * Red zone   -> right side
 */
#define RING_ARC_START_DEG         45.0f
#define RING_ARC_END_DEG          315.0f

/*
 * Marker appearance.
 */
#define RING_MARKER_OUTER_RADIUS      6
#define RING_MARKER_INNER_RADIUS      3

/*
 * Text placement inside the gauge.
 */
#define RING_LABEL_Y_OFFSET         (-28)
#define RING_VALUE_Y_OFFSET          (-2)

/*
 * RGB565 colors.
 */
#define COLOR_BLACK               0x0000
#define COLOR_WHITE               0xFFFF

#define COLOR_BLUE                0x001F
#define COLOR_CYAN                0x07FF
#define COLOR_GREEN               0x07E0
#define COLOR_YELLOW              0xFFE0
#define COLOR_ORANGE              0xFD20
#define COLOR_RED                 0xF800

#define DEG_TO_RAD                (M_PI / 180.0f)

/******************************************************************************
 * Private functions
 *****************************************************************************/

/*
 * Linear interpolation between two RGB565 colors.
 */
static uint16_t LerpColor(
    uint16_t c1,
    uint16_t c2,
    float t
)
{
    uint8_t r1 = (c1 >> 11) & 0x1F;
    uint8_t g1 = (c1 >> 5)  & 0x3F;
    uint8_t b1 =  c1        & 0x1F;

    uint8_t r2 = (c2 >> 11) & 0x1F;
    uint8_t g2 = (c2 >> 5)  & 0x3F;
    uint8_t b2 =  c2        & 0x1F;

    int16_t r =
        r1 + (int16_t)((float)(r2 - r1) * t);

    int16_t g =
        g1 + (int16_t)((float)(g2 - g1) * t);

    int16_t b =
        b1 + (int16_t)((float)(b2 - b1) * t);

    return ((uint16_t)r << 11) |
           ((uint16_t)g << 5)  |
           (uint16_t)b;
}

/*
 * Convert arc angle to rainbow gradient color.
 */
static uint16_t ColorFromAngle(
    float angle
)
{
    float t =
        (RING_ARC_END_DEG - angle) /
        (RING_ARC_END_DEG - RING_ARC_START_DEG);

    if(t < 0.0f)
    {
        t = 0.0f;
    }

    if(t > 1.0f)
    {
        t = 1.0f;
    }

    if(t < 0.20f)
    {
        return LerpColor(
            COLOR_BLUE,
            COLOR_CYAN,
            t / 0.20f
        );
    }

    if(t < 0.40f)
    {
        return LerpColor(
            COLOR_CYAN,
            COLOR_GREEN,
            (t - 0.20f) / 0.20f
        );
    }

    if(t < 0.60f)
    {
        return LerpColor(
            COLOR_GREEN,
            COLOR_YELLOW,
            (t - 0.40f) / 0.20f
        );
    }

    if(t < 0.80f)
    {
        return LerpColor(
            COLOR_YELLOW,
            COLOR_ORANGE,
            (t - 0.60f) / 0.20f
        );
    }

    return LerpColor(
        COLOR_ORANGE,
        COLOR_RED,
        (t - 0.80f) / 0.20f
    );
}

/*
 * Convert value to normalized range.
 *
 * Example:
 * 400 ppm  -> 0.0
 * 2000 ppm -> 1.0
 */
static float RingGauge_NormalizeValue(
    const RingGauge_t* gauge
)
{
    float percent =
        (gauge->value - gauge->minValue) /
        (gauge->maxValue - gauge->minValue);

    if(percent < 0.0f)
    {
        percent = 0.0f;
    }

    if(percent > 1.0f)
    {
        percent = 1.0f;
    }

    return percent;
}

/*
 * Convert normalized value to gauge angle.
 *
 * 400 ppm  -> blue zone (left)
 * 2000 ppm -> red zone (right)
 */
static float RingGauge_PercentToAngle(
    float percent
)
{
    return
        RING_ARC_END_DEG -
        percent *
        (
            RING_ARC_END_DEG -
            RING_ARC_START_DEG
        );
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

void RingGauge_DrawTextLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const RingGauge_t* gauge,
    const char* label,
    const char* valueText
)
{
    const int16_t labelY =
        gauge->centerY +
        RING_LABEL_Y_OFFSET;

    const int16_t valueY =
        gauge->centerY +
        RING_VALUE_Y_OFFSET;

    if(y >= labelY &&
       y < labelY + 32)
    {
        uint16_t textWidth =
            GFX_GetStringWidth(
                label,
                &FreeSans12pt7b
            );

        GFX_DrawStringLine(
            y - labelY,
            lineBuffer,
            width,
            label,
            gauge->centerX - textWidth / 2,
            COLOR_WHITE,
            &FreeSans12pt7b
        );
    }

    if(y >= valueY &&
       y < valueY + 32)
    {
        uint16_t textWidth =
            GFX_GetStringWidth(
                valueText,
                &FreeSans12pt7b
            );

        GFX_DrawStringLine(
            y - valueY,
            lineBuffer,
            width,
            valueText,
            gauge->centerX - textWidth / 2,
            COLOR_WHITE,
            &FreeSans12pt7b
        );
    }
}

void RingGauge_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const RingGauge_t* gauge
)
{
    const int32_t dy =
        (int32_t)y - gauge->centerY;

    const int32_t outerR =
        gauge->radius;

    const int32_t innerR =
        gauge->radius - gauge->thickness;

    const int32_t outerR2 =
        outerR * outerR;

    const int32_t innerR2 =
        innerR * innerR;

    const float percent =
        RingGauge_NormalizeValue(
            gauge
        );

    const float markerAngle =
        RingGauge_PercentToAngle(
            percent
        );

    const float markerRad =
        (markerAngle - 90.0f) *
        DEG_TO_RAD;

    const float markerRadiusPos =
        gauge->radius -
        gauge->thickness / 2.0f;

    const int16_t markerX =
        gauge->centerX +
        (int16_t)(
            markerRadiusPos *
            cosf(markerRad)
        );

    const int16_t markerY =
        gauge->centerY -
        (int16_t)(
            markerRadiusPos *
            sinf(markerRad)
        );

    for(uint16_t x = 0; x < width; x++)
    {
        const int32_t dx =
            (int32_t)x - gauge->centerX;

        const int32_t dist2 =
            dx * dx +
            dy * dy;

        /*
         * Draw rainbow arc.
         */
        if(dist2 <= outerR2 &&
           dist2 >= innerR2)
        {
            float angle =
                atan2f(
                    -(float)dy,
                    (float)dx
                ) *
                180.0f /
                M_PI;

            angle += 90.0f;

            if(angle < 0.0f)
            {
                angle += 360.0f;
            }

            if(angle >= RING_ARC_START_DEG &&
               angle <= RING_ARC_END_DEG)
            {
                lineBuffer[x] =
                    ColorFromAngle(
                        angle
                    );
            }
        }

        /*
         * Draw marker above arc.
         */
        const int16_t mdx =
            x - markerX;

        const int16_t mdy =
            y - markerY;

        const int32_t markerDist2 =
            mdx * mdx +
            mdy * mdy;

        if(markerDist2 <=
           RING_MARKER_OUTER_RADIUS *
           RING_MARKER_OUTER_RADIUS)
        {
            lineBuffer[x] = COLOR_WHITE;
        }

        if(markerDist2 <=
           RING_MARKER_INNER_RADIUS *
           RING_MARKER_INNER_RADIUS)
        {
            lineBuffer[x] = COLOR_BLACK;
        }
    }
}
