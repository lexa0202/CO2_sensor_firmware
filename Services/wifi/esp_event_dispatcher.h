#ifndef ESP_EVENT_DISPATCHER_H
#define ESP_EVENT_DISPATCHER_H

#include "esp_at.h"

/* ============================================================
 *                          TYPES
 * ============================================================ */

typedef void (*ESP_EventHandler_t)(ESP_AT_State_t ev);

/* ============================================================
 *                            API
 * ============================================================ */

void ESP_EventDispatcher_Register(ESP_EventHandler_t handler);
void ESP_EventDispatcher_Process(void);

#endif /* ESP_EVENT_DISPATCHER_H */
