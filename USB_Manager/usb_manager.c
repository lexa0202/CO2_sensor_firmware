#include "usb_manager.h"
#include "usb_device.h"
#include "fatfs.h"
#include "main.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern FATFS SDFatFS;

static bool msc_active = false;

static bool USB_IsConfigured(void)
{
    return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);
}

void USB_Manager_Process(void)
{
    bool configured = USB_IsConfigured();

    /* ПК подключился */
    if (configured && !msc_active)
    {
        f_mount(NULL, "", 0);   // SD отдаём ПК
        msc_active = true;
    }

    /* ПК отключился */
    if (!configured && msc_active)
    {
        f_mount(&SDFatFS, "", 1);   // SD возвращаем себе
        msc_active = false;
    }
}

bool USB_IsActive(void)
{
    return msc_active;
}
