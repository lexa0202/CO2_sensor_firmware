#ifndef USB_MANAGER_H
#define USB_MANAGER_H

#include <stdbool.h>

void USB_EnterMSC(void);
void USB_ExitMSC(void);
bool USB_IsBusy(void);
void USB_Manager_Init(void);
void USB_Manager_Process(void);
bool USB_IsActive(void);

#endif
