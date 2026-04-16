#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

typedef enum
{
    WIFI_STATUS_IDLE = 0,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_ERROR
} WIFI_Status_t;

void WIFI_Manager_Init(void);
void WIFI_Manager_Process(void);

WIFI_Status_t WIFI_Manager_GetStatus(void);
const char* WIFI_Manager_GetIP(void);

#endif
