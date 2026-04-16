#include "time_service.h"
#include "main.h"

uint32_t Time_GetMs(void)
{
    return HAL_GetTick();
}
