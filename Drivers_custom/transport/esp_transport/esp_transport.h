#ifndef ESP_TRANSPORT_H
#define ESP_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>

void ESP_Transport_PowerOn(void);
void ESP_Transport_PowerOff(void);
void ESP_Transport_Init(void);
void ESP_Transport_Process(void);
void ESP_Transport_Send(const char *cmd);

/* получить строку (например "OK", "ERROR", "+IPD") */
bool ESP_Transport_GetLine(char *out, uint16_t maxLen);

#endif
