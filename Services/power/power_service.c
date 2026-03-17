#include "power_service.h"
#include "main.h"

static DeviceMode_t deviceMode = DEVICE_MODE_NORMAL;

void Power_Service_Init(void)
{
    if(HAL_GPIO_ReadPin(USB_VBUS_GPIO_Port, USB_VBUS_Pin))
        deviceMode = DEVICE_MODE_USB;
    else
        deviceMode = DEVICE_MODE_NORMAL;
}

void Power_Service_Process(void)
{
    /* позже добавить детект батареи */
}

DeviceMode_t Power_GetDeviceMode(void)
{
    return deviceMode;
}
