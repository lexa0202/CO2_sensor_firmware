#include "usb_service.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "storage_service.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static bool mscActive = false;

static bool USB_IsConfigured(void)
{
    return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);
}

void USB_Service_Init(void)
{
    MX_USB_DEVICE_Init();
}

void USB_Service_Process(void)
{
    bool configured = USB_IsConfigured();

    /* ПК подключился */
    if (configured && !mscActive)
    {
        Storage_Service_SetOwner(STORAGE_OWNER_USB);
        mscActive = true;
    }

    /* ПК отключился */
    if (!configured && mscActive)
    {
        Storage_Service_SetOwner(STORAGE_OWNER_APP);
        mscActive = false;
    }
}

bool USB_Service_IsActive(void)
{
    return mscActive;
}
