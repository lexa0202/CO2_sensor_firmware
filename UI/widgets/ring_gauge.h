#ifndef RING_GAUGE_H
#define RING_GAUGE_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Types
 *****************************************************************************/

/*
 * Generic circular gauge widget.
 *
 * Can be used for:
 * - CO2 concentration
 * - temperature
 * - humidity
 * - pressure
 * - any normalized value
 */
typedef struct
{
    int16_t centerX;
    int16_t centerY;

    uint16_t radius;
    uint16_t thickness;

    float value;
    float minValue;
    float maxValue;

    uint16_t foregroundColor;
    uint16_t backgroundColor;

    const char* label;

} RingGauge_t;

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Render one display line of gauge graphics.
 */
void RingGauge_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const RingGauge_t* gauge
);

/*
 * Render centered gauge text.
 */
void RingGauge_DrawTextLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const RingGauge_t* gauge,
    const char* label,
    const char* valueText
);

/*
 * Get current marker position on the gauge arc.
 */
void RingGauge_GetMarkerPosition(
    const RingGauge_t* gauge,
    int16_t* markerX,
    int16_t* markerY
);

#endif /* RING_GAUGE_H */
