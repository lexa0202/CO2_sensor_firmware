#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ff.h"

typedef enum
{
    STORAGE_OWNER_APP = 0,
    STORAGE_OWNER_USB
} StorageOwner_t;

void Storage_Service_Init(void);
void Storage_Service_SetOwner(StorageOwner_t owner);
StorageOwner_t Storage_Service_GetOwner(void);

bool Storage_Service_IsAvailable(void);

/* Безопасное открытие файла */
FRESULT Storage_Open(FIL *file, const char *path, BYTE mode);
FRESULT Storage_Read(FIL *file, void *buff, UINT btr, UINT *br);
FRESULT Storage_Close(FIL *file);

#endif
