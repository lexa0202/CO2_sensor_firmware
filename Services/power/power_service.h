#ifndef POWER_SERVICE_H
#define POWER_SERVICE_H

#include <stdint.h>

typedef enum
{
    DEVICE_MODE_USB = 0, // connect to PC
    DEVICE_MODE_NORMAL, // 5V External source via USB
    DEVICE_MODE_BATTERY //5V from battery

} DeviceMode_t;

void Power_Service_Init(void);
void Power_Service_Process(void);

DeviceMode_t Power_GetDeviceMode(void);

#endif
