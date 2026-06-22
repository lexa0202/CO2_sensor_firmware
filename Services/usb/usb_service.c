#include "usb_service.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "storage_service.h"
#include "debug_console.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern void USB_Storage_SetReady(uint8_t ready);

static bool mscActive = false;

static bool USB_IsConfigured(void)
{
    return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);
}

void USB_Service_Init(void)
{
    MX_USB_DEVICE_Init();
    USB_Storage_SetReady(0);
}

void USB_Service_Process(void)
{
    static uint32_t usb_connect_time = 0;
    bool configured = USB_IsConfigured();

    // Отключение USB
    if (!configured && mscActive) {
        USB_Storage_SetReady(0);
        Storage_Service_SetOwner(STORAGE_OWNER_APP);
        mscActive = false;
        usb_connect_time = 0;
        return;
    }

    // Подключение USB
    if (configured && !mscActive) {
        if (usb_connect_time == 0) {
            usb_connect_time = HAL_GetTick();
            return;
        }

        // Ждем 50 мс, чтобы USB стабилизировался
        if (HAL_GetTick() - usb_connect_time < 50) {
            return;
        }

        Storage_Service_SetOwner(STORAGE_OWNER_USB);

        // Небольшая задержка для unmount
        HAL_Delay(20);

        USB_Storage_SetReady(1);
        Debug_Printf("USB READY TICK=%lu\r\n", HAL_GetTick());
        mscActive = true;
    }
}

bool USB_Service_IsActive(void)
{
    return mscActive;
}
