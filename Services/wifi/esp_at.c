#include "esp_at.h"
#include "esp_transport.h"
#include <string.h>
#include "debug_console.h"

#define ESP_EVENT_QUEUE_SIZE 8

static ESP_AT_State_t event_queue[ESP_EVENT_QUEUE_SIZE];
static uint8_t event_head = 0;
static uint8_t event_tail = 0;

static ESP_AT_State_t state = ESP_AT_IDLE;

static char line[128];
static char ip_addr[32];

static void event_push(ESP_AT_State_t ev)
{
    uint8_t next = (event_head + 1) % ESP_EVENT_QUEUE_SIZE;

    if(next != event_tail) // защита от overflow
    {
        event_queue[event_head] = ev;
        event_head = next;
    }
    // else: очередь переполнена → событие потеряно (можно логировать)
}

const char* ESP_AT_GetIP(void)
{
    return ip_addr;
}

void ESP_AT_Init(void)
{
    state = ESP_AT_IDLE;
    event_head = 0;
    event_tail = 0;
}

ESP_AT_State_t ESP_AT_GetEvent(void)
{
    if(event_tail == event_head)
        return ESP_AT_IDLE;

    ESP_AT_State_t ev = event_queue[event_tail];
    event_tail = (event_tail + 1) % ESP_EVENT_QUEUE_SIZE;

    return ev;
}

ESP_AT_State_t ESP_AT_GetState(void)
{
    return state;
}

void ESP_AT_Reset(void)
{
    state = ESP_AT_IDLE;
    event_head = 0;
    event_tail = 0;
    ip_addr[0] = '\0';
}

bool ESP_AT_IsBusy(void)
{
    return state == ESP_AT_WAIT_RESPONSE;
}

void ESP_AT_Process(void)
{
    while(ESP_Transport_GetLine(line, sizeof(line)))
    {
        Debug_Printf("ESP: %s\r\n", line);

        if(strstr(line, "WIFI GOT IP"))
        {
            event_push(ESP_AT_EVENT_GOT_IP);
        }
        else if(strcmp(line, "OK") == 0)
        {
            state = ESP_AT_IDLE;          // 🔴 ВЕРНУТЬ!
            event_push(ESP_AT_OK);
        }
        else if(strstr(line, "ERROR") || strstr(line, "FAIL"))
        {
            state = ESP_AT_IDLE;          // 🔴 ТОЖЕ!
            event_push(ESP_AT_ERROR);
        }
        else if(strstr(line, "WIFI DISCONNECT"))
        {
            event_push(ESP_AT_EVENT_DISCONNECT);
        }
        else if(strstr(line, "busy"))
        {
            // TODO
        }
        else if(strstr(line, "WIFI CONNECTED"))
        {
            event_push(ESP_AT_EVENT_CONNECTED);
        }
        else if(strstr(line, "+CIFSR:STAIP"))
        {
            char *start = strchr(line, '"');
            char *end   = strrchr(line, '"');

            if(start && end && end > start)
            {
                size_t len = end - start - 1;

                if(len < sizeof(ip_addr))
                {
                    strncpy(ip_addr, start + 1, len);
                    ip_addr[len] = '\0';

                    event_push(ESP_AT_EVENT_IP_PARSED);
                }
            }
        }
        // TCP CONNECT
        else if(strcmp(line, "CONNECT") == 0)
        {
            event_push(ESP_AT_EVENT_TCP_CONNECT);
        }
        // TCP CLOSED
        else if(strstr(line, "CLOSED"))
        {
            event_push(ESP_AT_EVENT_TCP_CLOSED);
        }
        else if(strchr(line, '>') != NULL)
        {
            event_push(ESP_AT_EVENT_SEND_PROMPT);
        }
        else if(strstr(line, "SEND OK"))
        {
            event_push(ESP_AT_EVENT_SEND_OK);
        }
        else if(strstr(line, "SEND FAIL"))
        {
            event_push(ESP_AT_EVENT_SEND_FAIL);
        }
        else
        {
            // неизвестные строки игнорируем
        }
    }
}

bool ESP_AT_Send(const char* cmd)
{
    if(state == ESP_AT_WAIT_RESPONSE)
        return false;

    ESP_Transport_Send(cmd);
    state = ESP_AT_WAIT_RESPONSE;

    return true;
}
