#ifndef SCREEN_USB_H
#define SCREEN_USB_H

/******************************************************************************
 * screen_usb.h
 *
 * USB Mass Storage mode screen API.
 *
 ******************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Render one scanline of the USB screen.
 */
void ScreenUsb_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
);

/*
 * Draw the full USB screen synchronously.
 */
void ScreenUsb_Draw(void);

#endif /* SCREEN_USB_H */
