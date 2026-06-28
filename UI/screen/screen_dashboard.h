#ifndef SCREEN_DASHBOARD_H
#define SCREEN_DASHBOARD_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Update dashboard animations.
 *
 * Currently used for CO2 sensor warmup animation.
 */
void ScreenDashboard_Update(void);

/*
 * Render one display line of dashboard screen.
 */
void ScreenDashboard_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
);

#endif /* SCREEN_DASHBOARD_H */
