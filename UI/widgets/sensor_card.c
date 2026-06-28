/******************************************************************************
 * sensor_card.c
 *
 * Dashboard sensor card widget.
 ******************************************************************************/

#include "sensor_card.h"

/******************************************************************************
 * Configuration
 *****************************************************************************/

/*
 * Corner radius.
 */
#define CARD_RADIUS    8

/******************************************************************************
 * Public functions
 *****************************************************************************/

void SensorCard_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t lineWidth,
    const SensorCard_t* card
)
{
    if(y < card->y ||
       y >= card->y + card->height)
    {
        return;
    }

    uint16_t localY =
        y - card->y;

    for(uint16_t x = card->x;
        x < card->x + card->width;
        x++)
    {
        uint16_t localX =
            x - card->x;

        uint8_t drawPixel = 1;

        /*
         * Rounded corners.
         */
        if(localX < CARD_RADIUS &&
           localY < CARD_RADIUS)
        {
            int16_t dx =
                CARD_RADIUS - localX;

            int16_t dy =
                CARD_RADIUS - localY;

            drawPixel =
                (dx * dx + dy * dy) <=
                (CARD_RADIUS * CARD_RADIUS);
        }

        if(localX >= card->width - CARD_RADIUS &&
           localY < CARD_RADIUS)
        {
            int16_t dx =
                localX -
                (card->width - CARD_RADIUS - 1);

            int16_t dy =
                CARD_RADIUS - localY;

            drawPixel =
                (dx * dx + dy * dy) <=
                (CARD_RADIUS * CARD_RADIUS);
        }

        if(localX < CARD_RADIUS &&
           localY >= card->height - CARD_RADIUS)
        {
            int16_t dx =
                CARD_RADIUS - localX;

            int16_t dy =
                localY -
                (card->height - CARD_RADIUS - 1);

            drawPixel =
                (dx * dx + dy * dy) <=
                (CARD_RADIUS * CARD_RADIUS);
        }

        if(localX >= card->width - CARD_RADIUS &&
           localY >= card->height - CARD_RADIUS)
        {
            int16_t dx =
                localX -
                (card->width - CARD_RADIUS - 1);

            int16_t dy =
                localY -
                (card->height - CARD_RADIUS - 1);

            drawPixel =
                (dx * dx + dy * dy) <=
                (CARD_RADIUS * CARD_RADIUS);
        }

        if(!drawPixel)
        {
            continue;
        }

        uint8_t border =
            (localX == 0) ||
            (localY == 0) ||
            (localX == card->width - 1) ||
            (localY == card->height - 1);

        lineBuffer[x] =
            border ?
            card->borderColor :
            card->backgroundColor;
    }
}
