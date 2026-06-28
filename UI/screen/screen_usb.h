#ifndef SCREEN_USB_H
#define SCREEN_USB_H

#include <stdint.h>

void ScreenUsb_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
);
void ScreenUsb_Draw(void);

#endif
