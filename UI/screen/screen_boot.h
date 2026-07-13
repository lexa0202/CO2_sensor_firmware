#ifndef SCREEN_BOOT_H
#define SCREEN_BOOT_H

/******************************************************************************
 * screen_boot.h
 *
 * Boot splash screen API.
 *
 * Shows the product logo, title and a refresh wave animation before
 * handing control back to the dashboard.
 *
 ******************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Reset boot screen timing and animation state.
 */
void ScreenBoot_Reset(void);

/*
 * Advance boot animation once per completed frame.
 */
void ScreenBoot_Update(void);

/*
 * Returns 1 when splash and wave animation are finished.
 */
uint8_t ScreenBoot_IsFinished(void);

/*
 * Render one scanline of the boot screen.
 */
void ScreenBoot_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
);

/*
 * Render the full boot frame synchronously.
 */
void ScreenBoot_DrawFullFrame(void);

#endif /* SCREEN_BOOT_H */
