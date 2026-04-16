#include "wifi_manager.h"
#include "tcp_client.h"
#include "esp_at.h"
#include "time_service.h"
#include <string.h>
#include <stdio.h>

#define WIFI_SSID "Light Side"
#define WIFI_PASS "St@rW@rs_LightSide1"

typedef enum
{
    WIFI_STATE_IDLE = 0,
    WIFI_STATE_AT,
    WIFI_STATE_WAIT_AT,
    WIFI_STATE_SET_MODE,
    WIFI_STATE_WAIT_MODE,
    WIFI_STATE_SET_MUX,
    WIFI_STATE_WAIT_MUX,
    WIFI_STATE_CONNECT,
    WIFI_STATE_WAIT_CONNECT,
    WIFI_STATE_WAIT_IP,
    WIFI_STATE_READY,
    WIFI_STATE_ERROR
} WifiState_t;

static WifiState_t state = WIFI_STATE_IDLE;
static uint32_t state_timer = 0;

static char wifi_ip[32] = {0};

void WIFI_Manager_Init(void)
{
    ESP_AT_Init();
    state = WIFI_STATE_AT;
}

WIFI_Status_t WIFI_Manager_GetStatus(void)
{
    switch(state)
    {
        case WIFI_STATE_READY: return WIFI_STATUS_CONNECTED;
        case WIFI_STATE_ERROR: return WIFI_STATUS_ERROR;
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

    ESP_AT_State_t ev;

    static bool ok = false;
    static bool error = false;
    static bool ip_parsed = false;
    static bool disconnected = false;
    static bool wifi_connected = false;
    static bool need_ip_request = false;

    while((ev = ESP_AT_GetEvent()) != ESP_AT_IDLE)
    {
    	switch(ev)
    	{
    	    case ESP_AT_OK: ok = true; break;
    	    case ESP_AT_ERROR: error = true; break;

    	    case ESP_AT_EVENT_GOT_IP:
    	        need_ip_request = true;
    	        break;
    	    case ESP_AT_EVENT_IP_PARSED: ip_parsed = true; break;
    	    case ESP_AT_EVENT_CONNECTED:
    	        wifi_connected = true;
    	        break;

    	    case ESP_AT_EVENT_DISCONNECT:
    	        wifi_connected = false;
    	        disconnected = true;
    	        break;

    	    default:
    	        break;
    	}
    }

    if(disconnected)
    {
        disconnected = false;
        wifi_connected = false;
        need_ip_request = false;
        wifi_ip[0] = '\0';

        state = WIFI_STATE_CONNECT;
    }

    switch(state)
    {
		case WIFI_STATE_AT:
			if(!ESP_AT_IsBusy() && ESP_AT_Send("AT\r\n"))
			{
				state = WIFI_STATE_WAIT_AT;
				state_timer = Time_GetMs();
			}
			break;

        case WIFI_STATE_WAIT_AT:
            if(ok)
            {
            	ok = false;
            	state = WIFI_STATE_SET_MODE;
            }
            else if(error)
            {
            	error = false;
                state = WIFI_STATE_AT;
            }
            else if(Time_GetMs() - state_timer > 2000)
            {
                state = WIFI_STATE_AT;
            }
            break;

        case WIFI_STATE_SET_MODE:
        	if(!ESP_AT_IsBusy() && ESP_AT_Send("AT+CWMODE=1\r\n"))
            {
                state = WIFI_STATE_WAIT_MODE;
                state_timer = Time_GetMs();
            }
            break;

        case WIFI_STATE_WAIT_MODE:
        	if(ok)
            {
        		ok = false;
        		state = WIFI_STATE_SET_MUX;
            }
            else if(error || Time_GetMs() - state_timer > 2000)
            {
            	error = false;
            	state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_SET_MUX:
        	if(!ESP_AT_IsBusy() && ESP_AT_Send("AT+CIPMUX=0\r\n"))
            {
                state = WIFI_STATE_WAIT_MUX;
                state_timer = Time_GetMs();
            }
            break;

        case WIFI_STATE_WAIT_MUX:
        	if(ok)
            {
        		ok = false;
        		state = WIFI_STATE_CONNECT;
            }
            else if(error || Time_GetMs() - state_timer > 2000)
            {
            	error = false;
            	state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_CONNECT:
        {
            if(wifi_connected)
            {
                // уже подключены → сразу ждём IP
                state = WIFI_STATE_WAIT_CONNECT;
                break;
            }

            char cmd[128];
            snprintf(cmd, sizeof(cmd),
                     "AT+CWJAP=\"%s\",\"%s\"\r\n",
                     WIFI_SSID, WIFI_PASS);

            if(ESP_AT_Send(cmd))
            {
                state = WIFI_STATE_WAIT_CONNECT;
                state_timer = Time_GetMs();
            }
            break;
        }

        case WIFI_STATE_WAIT_CONNECT:

            if(need_ip_request && !ESP_AT_IsBusy())
            {
                if(ESP_AT_Send("AT+CIFSR\r\n"))
                {
                    need_ip_request = false;
                    state = WIFI_STATE_WAIT_IP;
                    state_timer = Time_GetMs();
                }
            }
            else if(Time_GetMs() - state_timer > 30000)
            {
                state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_WAIT_IP:
            if(ip_parsed)
            {
                ip_parsed = false;

                const char* ip = ESP_AT_GetIP();

                if(ip && ip[0] != '\0')
                {
                    strncpy(wifi_ip, ip, sizeof(wifi_ip) - 1);
                    wifi_ip[sizeof(wifi_ip) - 1] = '\0';

                    state = WIFI_STATE_READY;
                }
                else
                {
                    state = WIFI_STATE_ERROR;
                }
            }
            else if(Time_GetMs() - state_timer > 2000)
            {
                state = WIFI_STATE_ERROR;
            }
            break;

        case WIFI_STATE_ERROR:
        default:
            break;
    }

    if(state == WIFI_STATE_ERROR)
    {
        static uint32_t retry_timer = 0;

        if(Time_GetMs() - retry_timer > 5000)
        {
            retry_timer = Time_GetMs();

            // мягкий рестарт логики, НЕ ESP
            ESP_AT_Reset();
            state = WIFI_STATE_AT;
        }
    }
}
