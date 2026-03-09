#include "screen_manager.h"
#include "lcd.h"
#include "storage_service.h"
#include "ff.h"
#include "animation_engine.h"
#include "font16x24.h"
#include "text_renderer.h"
#include "usb_icon.h"

#define SCREEN_TARGET_FPS   30
#define FRAME_PERIOD_MS     (1000 / SCREEN_TARGET_FPS)
// -------------text render------------------------
#define USB_TEXT_COLOR   0xFFFF
#define USB_BG_COLOR     0x0000
#define USB_LINE_SPACING 6
//-------------------------------------------------
//text render
static const char* usbLine1 = "USB is connected";
static const char* usbLine2 = "to PC";
//-------------------------------------------------
static uint8_t defaultImageDrawn = 0;
static uint8_t usbScreenDrawn = 0;
static ScreenState_t screenState = SCREEN_IDLE;
static FIL rawFile;
static uint8_t rawBuffer[512];
static UINT rawBytesRead;
// line-based rendering
static uint16_t lineBuffer[LCD_WIDTH];
static uint16_t currentLine = 0;
static uint32_t lastTick = 0;
static uint8_t frameActive = 0;
//FPS control
static uint32_t frameTimer = 0;
static uint8_t frameInProgress = 0;
//usb_icon


void Screen_Init(void)
{
	Animation_Init();
	lastTick = HAL_GetTick();
	frameTimer = 0;
	frameInProgress = 0;
    LCD_HardwareReset();
    ILI9341_Init();
}

void Screen_Black(void)
{
    ILI9341_Fill(0x0000);
    defaultImageDrawn = 0;
}

void Screen_ShowDefault(void)
{
    screenState = SCREEN_IDLE;
    defaultImageDrawn = 0;
}

static void Screen_StartRaw(const char* filename)
{
    if (Storage_Open(&rawFile, filename, FA_READ) != FR_OK)
    {
        ILI9341_Fill(0xFFE0);
        return;
    }

    if (f_size(&rawFile) != LCD_WIDTH * LCD_HEIGHT * 2UL)
    {
        ILI9341_Fill(0x001F);
        Storage_Close(&rawFile);
        return;
    }

    ILI9341_BeginFrame();
    screenState = SCREEN_DRAWING_RAW;
}

static void Screen_ProcessRaw(void)
{
    if (Storage_Read(&rawFile, rawBuffer, sizeof(rawBuffer), &rawBytesRead) != FR_OK)
    {
        ILI9341_Fill(0xF81F);
        ILI9341_EndFrame();
        Storage_Close(&rawFile);
        screenState = SCREEN_IDLE;
        return;
    }

    if (rawBytesRead == 0)
    {
        ILI9341_EndFrame();
        Storage_Close(&rawFile);
        screenState = SCREEN_IDLE;
        return;
    }

    ILI9341_PushData(rawBuffer, rawBytesRead);
}

void Screen_ShowUsb(void)
{
    screenState = SCREEN_USB;
    usbScreenDrawn = 0;
}

void Screen_Process(void)
{
	if (screenState == SCREEN_USB)
	    {
		if (usbScreenDrawn)
		{
		    return;
		}
		// Рассчитываем размеры блока
		uint16_t width1 = Text_GetStringWidth(usbLine1);
		uint16_t width2 = Text_GetStringWidth(usbLine2);

		int16_t startX1 = (LCD_WIDTH - width1) / 2;
		int16_t startX2 = (LCD_WIDTH - width2) / 2;

		if (startX1 < 0)
			startX1 = 0;

		if (startX2 < 0)
			startX2 = 0;

		uint16_t blockHeight = FONT_HEIGHT * 2 + USB_LINE_SPACING;

		 //Чуть выше центра
		uint16_t startY = (LCD_HEIGHT - blockHeight) / 2 - 20;

		for (uint16_t y = 0; y < LCD_HEIGHT; y++)
		{
			 //Заливка фона
			for (uint16_t x = 0; x < LCD_WIDTH; x++)
				lineBuffer[x] = USB_BG_COLOR;

			// Первая строка
			if (y >= startY && y < startY + FONT_HEIGHT)
			{
				Text_DrawStringLine(
					y - startY,
					lineBuffer,
					LCD_WIDTH,
					usbLine1,
					startX1,
					USB_TEXT_COLOR
				);
			}

			// Вторая строка
			if (y >= startY + FONT_HEIGHT + USB_LINE_SPACING &&
				y < startY + FONT_HEIGHT * 2 + USB_LINE_SPACING)
			{
				Text_DrawStringLine(
					y - (startY + FONT_HEIGHT + USB_LINE_SPACING),
					lineBuffer,
					LCD_WIDTH,
					usbLine2,
					startX2,
					USB_TEXT_COLOR
				);
			}

			ILI9341_WriteLine(y, lineBuffer);
		}
		uint16_t iconX = (LCD_WIDTH - USB_ICON_W) / 2;

		uint16_t iconY =
			startY +
			FONT_HEIGHT * 2 +
			USB_LINE_SPACING;

		ILI9341_DrawImage(
			iconX,
			iconY,
			USB_ICON_W,
			USB_ICON_H,
			usb_icon
		);


		usbScreenDrawn = 1;
		return;
	}


    /* 1. Если нужно нарисовать RAW */
    if (!defaultImageDrawn)
    {
        if (Storage_Service_IsAvailable())
        {
            Screen_StartRaw("IMAGE.RAW");
            defaultImageDrawn = 1;
        }
    }

    /* 2. Обработка RAW вывода */
    if (screenState == SCREEN_DRAWING_RAW)
    {
        Screen_ProcessRaw();
        return;
    }

    /* 4. Переход в режим анимации */
    if (screenState == SCREEN_IDLE)
    {
        screenState = SCREEN_ANIMATING;
        currentLine = 0;
        frameActive = 0;
        lastTick = HAL_GetTick();
    }

    /* 5. Анимация */
    if (screenState == SCREEN_ANIMATING)
    {
        uint32_t now = HAL_GetTick();
        uint32_t dt = now - lastTick;
        lastTick = now;

        frameTimer += dt;

        /* === Запуск нового кадра только по таймеру === */
        if (!frameInProgress)
        {
            if (frameTimer >= FRAME_PERIOD_MS)
            {
                frameTimer -= FRAME_PERIOD_MS;

                Animation_Update(FRAME_PERIOD_MS);

                currentLine = 0;
                frameInProgress = 1;
            }
            else
            {
                return;  // ждём следующего тика
            }
        }

        /* === Рисуем одну строку за итерацию === */
        Animation_RenderLine(currentLine, lineBuffer);
        ILI9341_WriteLine(currentLine, lineBuffer);

        currentLine++;

        if (currentLine >= LCD_HEIGHT)
        {
            frameInProgress = 0;
        }
    }
}

