#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <stdint.h>

typedef enum
{
    SCREEN_IDLE = 0,
    SCREEN_DRAWING_RAW,
    SCREEN_ANIMATING,
    SCREEN_USB
} ScreenState_t;

void Screen_Init(void);
void Screen_Process(void);
void Screen_ShowUsb(void);
void Screen_Black(void);
void Screen_ShowDefault(void);

#endif
