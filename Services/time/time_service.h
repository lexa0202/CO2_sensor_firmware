#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <stdint.h>
#include"main.h"

uint32_t Time_GetMs(void)
{
    return HAL_GetTick();
}

#endif
