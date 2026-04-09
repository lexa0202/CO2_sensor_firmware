#include "app.h"
#include "usb_service.h"
#include "screen_manager.h"
#include "wifi_manager.h"
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
    {
        App_EnterState(APP_STATE_USB_MSC);
    }
    else
    {
        App_EnterState(APP_STATE_UI);
    }
}

static void App_DebugOutput(void)
{
    static uint32_t lastDebug = 0;

    if (HAL_GetTick() - lastDebug < 1000)
        return;

    lastDebug = HAL_GetTick();

    const SensorData_t* data = SensorModel_Get();

    int t_i = (int)data->temperature;
    int t_f = (int)((data->temperature - t_i) * 100);

    int h_i = (int)data->humidity;
    int h_f = (int)((data->humidity - h_i) * 100);

    float p = SensorModel_GetPressureMMHg();
    int p_i = (int)p;
    int p_f = (int)((p - p_i) * 100);

    /* ===== CO2 STATUS ===== */
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
            snprintf(co2_buffer, sizeof(co2_buffer),
                     "CAL(%d)", data->co2);
            break;

        case SENSOR_CO2_STATUS_OK:
        default:
            snprintf(co2_buffer, sizeof(co2_buffer),
                     "%dppm", data->co2);
            break;
    }

    /* ===== WIFI STATUS ===== */

    WIFI_Status_t wifi = WIFI_Manager_GetStatus();
    const char* ip = WIFI_Manager_GetIP();

    const char* wifi_str = "IDLE";

    switch(wifi)
    {
        case WIFI_STATUS_CONNECTING:
            wifi_str = "CONNECTING";
            break;

        case WIFI_STATUS_CONNECTED:
            wifi_str = "CONNECTED";
            break;

        case WIFI_STATUS_ERROR:
            wifi_str = "ERROR";
            break;

        case WIFI_STATUS_IDLE:
        default:
            wifi_str = "IDLE";
            break;
    }

    /* ===== OUTPUT ===== */

    Debug_Printf(
        "T=%d.%02dC H=%d.%02d%% P=%d.%02dmmHg CO2=%s WIFI=%s IP=%s\r\n",
        t_i, t_f,
        h_i, h_f,
        p_i, p_f,
        co2_buffer,
        wifi_str,
		ip
    );
}

void App_Process(void)
{
    bool usbActive = USB_Service_IsActive();

    switch (currentState)
    {

    	case APP_STATE_USB_MSC:
    		if (!usbActive)
    		{
    			App_EnterState(APP_STATE_UI);
    		}
    		else
    		{
    			Screen_Process();
    		}
    		break;

        case APP_STATE_UI:
            if (usbActive)
            {
                App_EnterState(APP_STATE_USB_MSC);
            }
            else
            {
                Screen_Process();
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
