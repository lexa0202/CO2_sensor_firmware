#include "app.h"
#include "usb_service.h"
#include "screen_manager.h"
#include "wifi_manager.h"
#include "tcp_client.h"
#include "storage_service.h"
#include "sensor_model.h"
#include "debug_console.h"
#include "main.h"

static AppState_t currentState = APP_STATE_BOOT;
static AppState_t previousState = APP_STATE_BOOT;

static void App_EnterState(AppState_t newState)
{
    previousState = currentState;
    currentState = newState;

    switch (currentState)
    {
        case APP_STATE_USB_MSC:
            Screen_ShowUsb();
            break;

        case APP_STATE_UI:
            Screen_ShowDefault();
            break;

        case APP_STATE_BOOT:
        default:
            break;
    }
}

void App_Init(void)
{
    if (USB_Service_IsActive())
        App_EnterState(APP_STATE_USB_MSC);
    else
        App_EnterState(APP_STATE_UI);
}

static void App_DebugOutput(void)
{
    static uint32_t lastDebug = 0;
    if (HAL_GetTick() - lastDebug < 1000) return;
    lastDebug = HAL_GetTick();

    const SensorData_t* data = SensorModel_Get();

    int t_i = (int)data->temperature;
    int t_f = (int)((data->temperature - t_i) * 100);

    int h_i = (int)data->humidity;
    int h_f = (int)((data->humidity - h_i) * 100);

    float p = SensorModel_GetPressureMMHg();
    int p_i = (int)p;
    int p_f = (int)((p - p_i) * 100);

    /* ===== CO2 ===== */
    char co2_buffer[32];
    switch(data->co2_status)
    {
        case SENSOR_CO2_STATUS_WARMUP:
            snprintf(co2_buffer, sizeof(co2_buffer), "WARM");
            break;
        case SENSOR_CO2_STATUS_NO_RESPONSE:
            snprintf(co2_buffer, sizeof(co2_buffer), "N/A");
            break;
        case SENSOR_CO2_STATUS_OUT_OF_RANGE:
            snprintf(co2_buffer, sizeof(co2_buffer), "CAL(%d)", data->co2);
            break;
        default:
            snprintf(co2_buffer, sizeof(co2_buffer), "%dppm", data->co2);
            break;
    }

    /* ===== WIFI ===== */
    WIFI_Status_t wifi = WIFI_Manager_GetStatus();
    const char* ip = WIFI_Manager_GetIP();

    const char* wifi_str = "IDLE";
    switch(wifi)
    {
        case WIFI_STATUS_CONNECTING: wifi_str = "CONNECTING"; break;
        case WIFI_STATUS_CONNECTED:  wifi_str = "CONNECTED"; break;
        case WIFI_STATUS_ERROR:      wifi_str = "ERROR"; break;
        default: break;
    }

    /* ===== TCP ===== */
    TCP_State_t tcp = TCP_Client_GetState();

    const char* tcp_str = "-";
    switch(tcp)
    {
        case TCP_STATE_CONNECTING: tcp_str = "CONNECTING"; break;
        case TCP_STATE_CONNECTED:  tcp_str = "CONNECTED"; break;
        case TCP_STATE_SENDING:    tcp_str = "SENDING"; break;
        case TCP_STATE_ERROR:      tcp_str = "ERROR"; break;
        default: break;
    }

    /* ===== OUTPUT ===== */
    Debug_Printf(
        "T=%d.%02dC H=%d.%02d%% P=%d.%02dmmHg CO2=%s WIFI=%s IP=%s TCP=%s\r\n",
        t_i, t_f,
        h_i, h_f,
        p_i, p_f,
        co2_buffer,
        wifi_str,
        ip,
        tcp_str
    );
}

static void App_TcpTest(void)
{
    static bool started = false;
    static bool sent = false;

    if(!started && WIFI_Manager_GetStatus() == WIFI_STATUS_CONNECTED)
    {
        if(TCP_Client_Connect("example.com", 80))
        {
            started = true;
            Debug_Printf("TCP START\r\n");
        }
    }

    if(started && !sent && TCP_Client_IsConnected())
    {
        const char *msg =
            "GET / HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        if(TCP_Client_Send((const uint8_t*)msg, strlen(msg)))
        {
            sent = true;
            Debug_Printf("TCP SEND\r\n");
        }
    }
}

void App_Process(void)
{
    bool usbActive = USB_Service_IsActive();

    switch (currentState)
    {
        case APP_STATE_USB_MSC:
            if (!usbActive)
                App_EnterState(APP_STATE_UI);
            else
                Screen_Process();
            break;

        case APP_STATE_UI:
            if (usbActive)
            {
                App_EnterState(APP_STATE_USB_MSC);
            }
            else
            {
                Screen_Process();
                TCP_Client_Process();
                App_TcpTest();
                App_DebugOutput();
            }
            break;

        case APP_STATE_BOOT:
        default:
            break;
    }
}

AppState_t App_GetState(void)
{
    return currentState;
}
