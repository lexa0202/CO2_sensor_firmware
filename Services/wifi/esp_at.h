#ifndef ESP_AT_H
#define ESP_AT_H

#include <stdbool.h>

typedef enum
{
    ESP_AT_IDLE = 0,
    ESP_AT_WAIT_RESPONSE,

    ESP_AT_OK,          // ответ на команду (OK)
    ESP_AT_ERROR,       // ERROR / FAIL

    ESP_AT_EVENT_GOT_IP, // событие подключения к Wi-Fi
	ESP_AT_EVENT_IP_PARSED,   // IP из CIFSR
	ESP_AT_EVENT_DISCONNECT,
	ESP_AT_EVENT_CONNECTED,

	ESP_AT_EVENT_TCP_CONNECT,     // CONNECT
	ESP_AT_EVENT_TCP_CLOSED,      // CLOSED
	ESP_AT_EVENT_TCP_ERROR,        // ERROR при TCP

	ESP_AT_EVENT_SEND_PROMPT,   // throw TCP >
	ESP_AT_EVENT_SEND_OK,
	ESP_AT_EVENT_SEND_FAIL
} ESP_AT_State_t;

void ESP_AT_Init(void);
void ESP_AT_Process(void);
bool ESP_AT_IsBusy(void);
const char* ESP_AT_GetIP(void);
bool ESP_AT_Send(const char* cmd);
ESP_AT_State_t ESP_AT_GetState(void);
ESP_AT_State_t ESP_AT_GetEvent(void);
void ESP_AT_Reset(void);

#endif
