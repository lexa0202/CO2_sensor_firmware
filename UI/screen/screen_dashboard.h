#ifndef SCREEN_DASHBOARD_H
#define SCREEN_DASHBOARD_H

/******************************************************************************
 * screen_dashboard.h
 *
 * Main dashboard screen API.
 *
 * Renders the CO2 gauge, animated face and sensor cards.
 *
 ******************************************************************************/

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
void ScreenDashboard_Init(void);

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
