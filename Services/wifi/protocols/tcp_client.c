#include "tcp_client.h"
#include "esp_at.h"
#include "esp_transport.h"
#include "time_service.h"
#include <string.h>
#include <stdio.h>

static TCP_State_t state = TCP_STATE_IDLE;

static char host_buf[64];
static uint16_t port_buf = 0;

static const uint8_t* tx_data = NULL;
static uint16_t tx_len = 0;

static uint32_t state_timer = 0;

static bool cmd_sent = false;

void TCP_Client_Init(void)
{
    state = TCP_STATE_IDLE;
}

TCP_State_t TCP_Client_GetState(void)
{
    return state;
}

bool TCP_Client_IsConnected(void)
{
    return state == TCP_STATE_CONNECTED;
}

bool TCP_Client_Connect(const char* host, uint16_t port)
{
    if(state != TCP_STATE_IDLE)
        return false;

    strncpy(host_buf, host, sizeof(host_buf) - 1);
    port_buf = port;

    cmd_sent = false;   // 🔴 важно

    state = TCP_STATE_CONNECTING;
    return true;
}

bool TCP_Client_Send(const uint8_t* data, uint16_t len)
{
    if(state != TCP_STATE_CONNECTED)
        return false;

    tx_data = data;
    tx_len = len;

    state = TCP_STATE_SENDING;
    return true;
}

void TCP_Client_HandleEvent(ESP_AT_State_t ev)
{
    switch(ev)
    {
        case ESP_AT_EVENT_TCP_CONNECT:
            if(state == TCP_STATE_CONNECTING)
            {
                state = TCP_STATE_CONNECTED;
                cmd_sent = false;
            }
            break;

        case ESP_AT_EVENT_TCP_CLOSED:
        case ESP_AT_EVENT_TCP_ERROR:
            state = TCP_STATE_IDLE;
            break;

        case ESP_AT_EVENT_SEND_PROMPT:
            if(state == TCP_STATE_WAIT_PROMPT)
            {
                ESP_Transport_Send((const char*)tx_data);

                state = TCP_STATE_SENDING;
                state_timer = Time_GetMs();
            }
            break;

        case ESP_AT_EVENT_SEND_OK:
            if(state == TCP_STATE_SENDING)
            {
                state = TCP_STATE_CONNECTED;
                cmd_sent = false;
            }
            break;

        case ESP_AT_EVENT_SEND_FAIL:
            state = TCP_STATE_ERROR;
            break;

        default:
            break;
    }
}

void TCP_Client_Process(void)
{
    switch(state)
    {
    case TCP_STATE_CONNECTING:
    {
        if(!cmd_sent && !ESP_AT_IsBusy())
        {
            char cmd[128];
            snprintf(cmd, sizeof(cmd),
                     "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",
                     host_buf, port_buf);

            if(ESP_AT_Send(cmd))
            {
                cmd_sent = true;
                state_timer = Time_GetMs();
            }
        }
        break;
    }

		case TCP_STATE_SENDING:
		{
			if(!cmd_sent && !ESP_AT_IsBusy())
			{
				char cmd[32];
				snprintf(cmd, sizeof(cmd),
						 "AT+CIPSEND=%d\r\n", tx_len);

				if(ESP_AT_Send(cmd))
				{
					cmd_sent = true;
					state = TCP_STATE_WAIT_PROMPT;
					state_timer = Time_GetMs();
				}
			}
			break;
		}

        case TCP_STATE_CONNECTED:
        case TCP_STATE_IDLE:
        case TCP_STATE_ERROR:
        default:
            break;
    }

    // таймауты
    if(state == TCP_STATE_CONNECTING &&
       (Time_GetMs() - state_timer > 5000))
    {
        state = TCP_STATE_ERROR;
    }

    if(state == TCP_STATE_SENDING &&
       (Time_GetMs() - state_timer > 3000))
    {
        state = TCP_STATE_ERROR;
    }
}
