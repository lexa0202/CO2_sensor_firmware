#ifndef SENSOR_CARD_H
#define SENSOR_CARD_H

/******************************************************************************
 * sensor_card.h
 *
 * Compact sensor information card.
 *
 * Used by dashboard screen to display
 * temperature, humidity and pressure.
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Types
 *****************************************************************************/

typedef struct
{
    int16_t x;
    int16_t y;

    uint16_t width;
    uint16_t height;

    uint16_t borderColor;
    uint16_t backgroundColor;

} SensorCard_t;

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Render one scanline of a sensor card.
 */
void SensorCard_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,
    const SensorCard_t* card
);

#endif /* SENSOR_CARD_H */
