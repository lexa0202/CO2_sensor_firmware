#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_at.h"

typedef enum
{
    TCP_STATE_IDLE = 0,
    TCP_STATE_CONNECTING,
    TCP_STATE_CONNECTED,
	TCP_STATE_WAIT_PROMPT,
	TCP_STATE_SENDING,
    TCP_STATE_ERROR
} TCP_State_t;

void TCP_Client_Init(void);
void TCP_Client_Process(void);

bool TCP_Client_Connect(const char* host, uint16_t port);
bool TCP_Client_Send(const uint8_t* data, uint16_t len);
void TCP_Client_HandleEvent(ESP_AT_State_t ev);

bool TCP_Client_IsConnected(void);
TCP_State_t TCP_Client_GetState(void);

#endif
