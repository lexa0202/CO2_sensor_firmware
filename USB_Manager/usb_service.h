#ifndef USB_SERVICE_H
#define USB_SERVICE_H

#include <stdbool.h>

void USB_Service_Init(void);
void USB_Service_Process(void);

bool USB_Service_IsActive(void);

#endif
