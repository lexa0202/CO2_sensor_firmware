#include "app.h"
#include "usb_service.h"
#include "screen_manager.h"
#include "storage_service.h"

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
