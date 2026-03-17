#include "storage_service.h"
#include "fatfs.h"
#include "main.h"

static StorageOwner_t currentOwner = STORAGE_OWNER_APP;
static bool mounted = false;

void Storage_Service_Init(void)
{
    if (f_mount(&SDFatFS, "", 1) == FR_OK)
    {
        mounted = true;
        currentOwner = STORAGE_OWNER_APP;
    }
    else
    {
        mounted = false;
    }
}

void Storage_Service_SetOwner(StorageOwner_t owner)
{
    if (owner == currentOwner)
        return;

    if (owner == STORAGE_OWNER_USB)
    {
        f_mount(NULL, "", 0);
        mounted = false;
        currentOwner = STORAGE_OWNER_USB;
    }
    else
    {
        if (f_mount(&SDFatFS, "", 1) == FR_OK)
        {
            mounted = true;
            currentOwner = STORAGE_OWNER_APP;
        }
    }
}

StorageOwner_t Storage_Service_GetOwner(void)
{
    return currentOwner;
}

bool Storage_Service_IsAvailable(void)
{
    return (mounted && currentOwner == STORAGE_OWNER_APP);
}

FRESULT Storage_Open(FIL *file, const char *path, BYTE mode)
{
    if (!Storage_Service_IsAvailable())
        return FR_NOT_READY;

    return f_open(file, path, mode);
}

FRESULT Storage_Read(FIL *file, void *buff, UINT btr, UINT *br)
{
    if (!Storage_Service_IsAvailable())
        return FR_NOT_READY;

    return f_read(file, buff, btr, br);
}

FRESULT Storage_Close(FIL *file)
{
    return f_close(file);
}
