#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    APP_STATE_BOOT = 0,
    APP_STATE_USB_MSC,
    APP_STATE_UI
} AppState_t;

void App_Init(void);
void App_Process(void);

AppState_t App_GetState(void);

#endif
