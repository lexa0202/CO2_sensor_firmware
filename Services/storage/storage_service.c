#include "storage_service.h"
#include "fatfs.h"
#include "main.h"
#include "debug_console.h" // DELETE

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
    Debug_Printf("Storage: SetOwner from %d to %d\r\n", currentOwner, owner);

    if (owner == currentOwner)
        return;

    if (owner == STORAGE_OWNER_USB)
    {
        Debug_Printf("Storage: unmounting FATFS for USB\r\n");
        f_mount(NULL, "", 0);
        mounted = false;
        currentOwner = STORAGE_OWNER_USB;
        Debug_Printf("Storage: USB now owns SD (mounted=%d)\r\n", mounted);
    }
    else
    {
        Debug_Printf("Storage: mounting FATFS for APP\r\n");
        if (f_mount(&SDFatFS, "", 1) == FR_OK)
        {
            mounted = true;
            currentOwner = STORAGE_OWNER_APP;
            Debug_Printf("Storage: APP now owns SD (OK)\r\n");
        }
        else
        {
            Debug_Printf("Storage: APP mount FAILED\r\n");
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

FRESULT Storage_CloseAll(void)
{
    // В реальном проекте здесь нужно закрывать все открытые файлы
    // Пока сделаем минимальную реализацию - просто проверяем, открыт ли IMAGE.RAW
    // В будущем можно сделать массив открытых файлов

    // Для текущего проекта достаточно просто вернуть OK
    // Основной файл (IMAGE.RAW) закрывается автоматически в Screen_ProcessRaw
    return FR_OK;
}
