#include "time_service.h"
#include "wifi_manager.h"
#include "esp_at.h"
//#include "debug_console.h" // delete after release
#include <stdint.h>
#include <stdio.h>

#define WIFI_SSID     "Light Side"
#define WIFI_PASS     "St@rW@rs_LightSide1"

static uint32_t retry_timer = 0;
static uint8_t retry_count = 0;
static char wifi_ip[32] = {0};
static uint8_t ip_retry_count = 0;

typedef enum
{
    WIFI_STATE_IDLE = 0,
    WIFI_STATE_AT,
    WIFI_STATE_WAIT_AT,
    WIFI_STATE_SET_MODE,
    WIFI_STATE_WAIT_MODE,
    WIFI_STATE_CONNECT,
    WIFI_STATE_WAIT_CONNECT,

	WIFI_STATE_GET_IP,
	WIFI_STATE_WAIT_IP,
    WIFI_STATE_READY,
    WIFI_STATE_ERROR
} WifiState_t;

static WifiState_t state = WIFI_STATE_IDLE;

void WIFI_Manager_Init(void)
{
    ESP_AT_Init();
    state = WIFI_STATE_AT;
}

WIFI_Status_t WIFI_Manager_GetStatus(void)
{
    switch(state)
    {
        case WIFI_STATE_READY:
            return WIFI_STATUS_CONNECTED;

        case WIFI_STATE_ERROR:
            return WIFI_STATUS_ERROR;

        case WIFI_STATE_AT:
        case WIFI_STATE_WAIT_AT:
        case WIFI_STATE_SET_MODE:
        case WIFI_STATE_WAIT_MODE:
        case WIFI_STATE_CONNECT:
        case WIFI_STATE_WAIT_CONNECT:
            return WIFI_STATUS_CONNECTING;

        default:
            return WIFI_STATUS_IDLE;
    }
}

const char* WIFI_Manager_GetIP(void)
{
    return wifi_ip;
}

void WIFI_Manager_Process(void)
{
    ESP_AT_Process();

    static uint32_t state_timer = 0;
    ESP_AT_State_t resp = ESP_AT_GetState();   // ответы (OK / ERROR)

    ESP_AT_State_t ev; // events 8 in line

    while((ev = ESP_AT_GetEvent()) != ESP_AT_IDLE)
    {
        if(ev == ESP_AT_EVENT_DISCONNECT && state == WIFI_STATE_READY)
        {
            wifi_ip[0] = '\0';
            ESP_AT_Reset();
            state = WIFI_STATE_ERROR;
        }

        if(ev == ESP_AT_EVENT_GOT_IP && state == WIFI_STATE_WAIT_CONNECT)
        {
            state = WIFI_STATE_GET_IP;
        }

        if(ev == ESP_AT_EVENT_IP_PARSED && state == WIFI_STATE_WAIT_IP)
        {
            const char* ip = ESP_AT_GetIP();

            if(ip && ip[0] != '\0' && strcmp(ip, "0.0.0.0") != 0)
            {
                if(strcmp(wifi_ip, ip) != 0)
                {
                    strncpy(wifi_ip, ip, sizeof(wifi_ip) - 1);
                    wifi_ip[sizeof(wifi_ip) - 1] = '\0';
                }

                ip_retry_count = 0;
                state = WIFI_STATE_READY;
            }
            else
            {
                state = WIFI_STATE_ERROR;
            }
        }
    }

    switch(state)
    {
        case WIFI_STATE_AT:
            if(ESP_AT_Send("AT\r\n"))
            {
                state = WIFI_STATE_WAIT_AT;
                state_timer = Time_GetMs();
            }
            break;

        case WIFI_STATE_WAIT_AT:
            if(resp == ESP_AT_OK)
            {
                ESP_AT_Reset();
                state = WIFI_STATE_SET_MODE;
            }
            else if(resp == ESP_AT_ERROR)
            {
                ESP_AT_Reset();
                state = WIFI_STATE_ERROR;
            }
            else if(Time_GetMs() - state_timer > 2000)
            {
                state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_SET_MODE:
            if(ESP_AT_Send("AT+CWMODE=1\r\n"))
            {
                state = WIFI_STATE_WAIT_MODE;
                state_timer = Time_GetMs();
            }
            break;

        case WIFI_STATE_WAIT_MODE:
            if(resp == ESP_AT_OK)
            {
                ESP_AT_Reset();
                state = WIFI_STATE_CONNECT;
            }
            else if(resp == ESP_AT_ERROR)
            {
                ESP_AT_Reset();
                state = WIFI_STATE_ERROR;
            }
            else if(Time_GetMs() - state_timer > 2000)
            {
                state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_CONNECT:
        {
            char cmd[128];

            snprintf(cmd, sizeof(cmd),
                     "AT+CWJAP=\"%s\",\"%s\"\r\n",
                     WIFI_SSID, WIFI_PASS);

            if(ESP_AT_Send(cmd))
            {
                state = WIFI_STATE_WAIT_CONNECT;
                state_timer = Time_GetMs();
                ip_retry_count = 0;
            }
            break;
        }

        case WIFI_STATE_WAIT_CONNECT:
            if(resp == ESP_AT_ERROR)
            {
                ESP_AT_Reset();
                state = WIFI_STATE_ERROR;
            }
            else if(Time_GetMs() - state_timer > 30000)
            {
                state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_GET_IP:
            if(ESP_AT_Send("AT+CIFSR\r\n"))
            {
                state = WIFI_STATE_WAIT_IP;
                state_timer = Time_GetMs();
            }
            break;

        case WIFI_STATE_WAIT_IP:
            if(resp == ESP_AT_ERROR)
            {
                state = WIFI_STATE_ERROR;
            }
            else if(Time_GetMs() - state_timer > 2000)
            {
                if(ip_retry_count < 3)
                {
                    ip_retry_count++;
                    state = WIFI_STATE_GET_IP;
                }
                else
                {
                    state = WIFI_STATE_ERROR;
                }
            }
            break;

        case WIFI_STATE_READY:
            // тут только удержание состояния
            break;

        case WIFI_STATE_ERROR:
            if(Time_GetMs() - retry_timer > 5000)
            {
                retry_timer = Time_GetMs();
                retry_count++;

                ESP_AT_Reset();
                state = WIFI_STATE_AT;
            }
            break;

        default:
            break;
    }
}
