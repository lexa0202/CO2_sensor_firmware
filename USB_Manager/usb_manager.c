#include "usb_manager.h"
#include "sd_manager.h"
#include "usbd_storage_if.h"
#include "usb_device.h"
#include "fatfs.h"
#include "main.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern FATFS SDFatFS;

static bool usb_boot_state = false;
static bool usb_active = false;

bool USB_IsConnected(void)
{
    return (HAL_GPIO_ReadPin(USB_VBUS_GPIO_Port,
                             USB_VBUS_Pin) == GPIO_PIN_SET);
}

void USB_EnterMSC(void)
{
    if (usb_active)
        return;

    f_mount(NULL, "", 0);

    SD_WaitReady(1000);

    MX_USB_DEVICE_Init();

    usb_active = true;
}

void USB_ExitMSC(void)
{
    if (!usb_active)
        return;
// обрабатываем безопасное извлечение
    uint32_t timeout = HAL_GetTick();
    while (USB_IsBusy())
    {
        if (HAL_GetTick() - timeout > 2000)
            break;
    }

    USBD_Stop(&hUsbDeviceFS);
    USBD_DeInit(&hUsbDeviceFS);

    f_mount(&SDFatFS, "", 1);


    usb_active = false;
}

void USB_Manager_Init(void)
{
	usb_boot_state = USB_IsConnected();

    if (usb_boot_state)
        USB_EnterMSC();
    else
        SD_Init();
}

void USB_Manager_Process(void)
{
    bool usb_now = USB_IsConnected();

    if (usb_now && !usb_boot_state)
        USB_EnterMSC();

    if (!usb_now && usb_boot_state)
        USB_ExitMSC();

    usb_boot_state = usb_now;
}

bool USB_IsActive(void)
{
    return usb_active;
}
