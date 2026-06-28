/******************************************************************************
 * screen_manager.c
 *
 * Main display manager.
 *
 * Responsibilities:
 * - LCD initialization
 * - RAW image rendering
 * - Dashboard rendering
 * - Animation rendering
 * - USB screen handling
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "screen_manager.h"

#include <ili9341_driver.h>

#include "storage_service.h"
#include "animation_engine.h"
#include "power_service.h"

#include "screen_dashboard.h"
#include "screen_usb.h"

#include "ff.h"

/******************************************************************************
 * Configuration
 *****************************************************************************/

/*
 * Animation frame rate.
 */
#define SCREEN_TARGET_FPS      30U
#define FRAME_PERIOD_MS        (1000U / SCREEN_TARGET_FPS)

/*
 * Default splash image.
 */
#define DEFAULT_IMAGE_FILE     "IMAGE.RAW"

/*
 * Debug colors.
 */
#define COLOR_BLACK            0x0000
#define COLOR_YELLOW           0xFFE0
#define COLOR_BLUE             0x001F
#define COLOR_MAGENTA          0xF81F

/******************************************************************************
 * Static data
 *****************************************************************************/

static ScreenState_t screenState =
    SCREEN_DASHBOARD;

static uint8_t defaultImageDrawn =
    0;

static uint8_t usbScreenDrawn =
    0;

static FIL rawFile;

static uint8_t rawBuffer[512];

static UINT rawBytesRead;

static uint16_t lineBuffer[LCD_WIDTH];

static uint16_t currentLine =
    0;

static uint32_t lastTick =
    0;

static uint32_t frameTimer =
    0;

static uint8_t frameInProgress =
    0;

/******************************************************************************
 * Private functions
 *****************************************************************************/

/*
 * Start RAW image rendering.
 */
static void Screen_StartRaw(
    const char* filename
)
{
    ILI9341_Fill(COLOR_BLACK);

    if(Storage_Open(
           &rawFile,
           filename,
           FA_READ) != FR_OK)
    {
        ILI9341_Fill(COLOR_YELLOW);
        return;
    }

    if(f_size(&rawFile) !=
       LCD_WIDTH *
       LCD_HEIGHT *
       2UL)
    {
        ILI9341_Fill(COLOR_BLUE);

        Storage_Close(
            &rawFile
        );

        return;
    }

    ILI9341_BeginFrame();

    screenState =
        SCREEN_DRAWING_RAW;
}

/*
 * Continue RAW image rendering.
 */
static void Screen_ProcessRaw(void)
{
    /*
     * Abort if storage ownership
     * was taken by USB.
     */
    if(Storage_Service_GetOwner() !=
       STORAGE_OWNER_APP)
    {
        ILI9341_EndFrame();

        Storage_Close(
            &rawFile
        );

        screenState =
            SCREEN_DASHBOARD;

        return;
    }

    if(Storage_Read(
           &rawFile,
           rawBuffer,
           sizeof(rawBuffer),
           &rawBytesRead) != FR_OK)
    {
        ILI9341_Fill(
            COLOR_MAGENTA
        );

        ILI9341_EndFrame();

        Storage_Close(
            &rawFile
        );

        screenState =
            SCREEN_DASHBOARD;

        return;
    }

    if(rawBytesRead == 0)
    {
        ILI9341_EndFrame();

        Storage_Close(
            &rawFile
        );

        screenState =
            SCREEN_DASHBOARD;

        return;
    }

    ILI9341_PushData(
        rawBuffer,
        rawBytesRead
    );
}

/*
 * Render one dashboard line.
 */
static void Screen_ProcessDashboard(void)
{
    ScreenDashboard_RenderLine(
        currentLine,
        lineBuffer,
        LCD_WIDTH
    );

    ILI9341_WriteLine(
        currentLine,
        lineBuffer
    );

    currentLine++;

    if(currentLine >= LCD_HEIGHT)
    {
        currentLine = 0;

        ScreenDashboard_Update();
    }
}

/*
 * Render one animation line.
 */
static void Screen_ProcessAnimation(void)
{
    uint32_t now =
        HAL_GetTick();

    uint32_t dt =
        now - lastTick;

    lastTick =
        now;

    frameTimer += dt;

    if(!frameInProgress)
    {
        if(frameTimer >=
           FRAME_PERIOD_MS)
        {
            frameTimer -=
                FRAME_PERIOD_MS;

            Animation_Update(
                FRAME_PERIOD_MS
            );

            currentLine = 0;

            frameInProgress = 1;
        }
        else
        {
            return;
        }
    }

    Animation_RenderLine(
        currentLine,
        lineBuffer
    );

    ILI9341_WriteLine(
        currentLine,
        lineBuffer
    );

    currentLine++;

    if(currentLine >= LCD_HEIGHT)
    {
        frameInProgress = 0;
    }
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

void Screen_Init(void)
{
    Animation_Init();

    lastTick =
        HAL_GetTick();

    frameTimer =
        0;

    frameInProgress =
        0;

    LCD_HardwareReset();

    ILI9341_Init();

    ILI9341_Fill(
        COLOR_BLACK
    );
}

void Screen_Black(void)
{
    ILI9341_Fill(
        COLOR_BLACK
    );

    defaultImageDrawn =
        0;

    currentLine =
        0;

    frameInProgress =
        0;
}

void Screen_ShowDefault(void)
{
    screenState =
        SCREEN_DASHBOARD;

    defaultImageDrawn =
        0;

    usbScreenDrawn =
        0;

    frameInProgress =
        0;

    currentLine =
        0;
}

void Screen_ShowUsb(void)
{
    screenState =
        SCREEN_USB;

    usbScreenDrawn =
        0;

    frameInProgress =
        0;

    currentLine =
        0;
}

void Screen_Process(void)
{
    DeviceMode_t mode =
        Power_GetDeviceMode();

    (void)mode;

    /*
     * USB screen.
     */
    if(screenState ==
       SCREEN_USB)
    {
        if(!usbScreenDrawn)
        {
            ScreenUsb_Draw();

            usbScreenDrawn =
                1;
        }

        return;
    }

    /*
     * Splash image.
     */
    if(!defaultImageDrawn)
    {
        if(Storage_Service_IsAvailable())
        {
            Screen_StartRaw(
                DEFAULT_IMAGE_FILE
            );

            defaultImageDrawn =
                1;
        }
    }

    /*
     * RAW rendering.
     */
    if(screenState ==
       SCREEN_DRAWING_RAW)
    {
        Screen_ProcessRaw();
        return;
    }

    /*
     * Dashboard.
     */
    if(screenState ==
       SCREEN_DASHBOARD)
    {
        Screen_ProcessDashboard();
        return;
    }

    /*
     * Animation mode.
     */
    if(screenState ==
       SCREEN_ANIMATING)
    {
        Screen_ProcessAnimation();
    }
}
