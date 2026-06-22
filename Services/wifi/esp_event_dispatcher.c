#include "esp_event_dispatcher.h"

#include <stdint.h>

/* ============================================================
 *                          CONFIG
 * ============================================================ */

#define MAX_HANDLERS 8

/* ============================================================
 *                         HANDLERS
 * ============================================================ */

static ESP_EventHandler_t handlers[MAX_HANDLERS];
static uint8_t            handler_count = 0;

/* ============================================================
 *                          REGISTER
 * ============================================================ */

void ESP_EventDispatcher_Register(ESP_EventHandler_t handler)
{
    if (handler_count < MAX_HANDLERS)
    {
        handlers[handler_count++] = handler;
    }
}

/* ============================================================
 *                          PROCESS
 * ============================================================ */

void ESP_EventDispatcher_Process(void)
{
    ESP_AT_State_t ev;

    while ((ev = ESP_AT_GetEvent()) != ESP_AT_IDLE)
    {
        for (uint8_t i = 0; i < handler_count; i++)
        {
            handlers[i](ev);
        }
    }
}
