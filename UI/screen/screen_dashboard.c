/******************************************************************************
 * screen_dashboard.c
 *
 * Main dashboard screen.
 *
 * Layout:
 * - CO2 rainbow gauge with marker
 * - CO2 label, value and ppm unit
 * - animated face below the gauge text
 * - temperature / humidity / pressure cards
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "screen_dashboard.h"

#include <stdio.h>

#include "sensor_model.h"
#include "ring_gauge.h"
#include "sensor_card.h"
#include "face_animation.h"

#include "degree.h"
#include "temp.h"
#include "humidity.h"
#include "pressure.h"
#include "bitmap_renderer.h"

#include "gfx_text_renderer.h"

#include "FreeSans7pt8b.h"
#include "FreeSans9pt7b.h"
#include "FreeSansBold12pt7b.h"
#include "YandexSansDisplay_Bold12pt7b.h"
#include "YandexSansDisplay_Bold20pt7b.h"
#include "YandexSansDisplay_Light12pt7b.h"
#include "YandexSansDisplay_Light20pt7b.h"

/******************************************************************************
 * Configuration
 *****************************************************************************/

#define DASHBOARD_BG_COLOR           0x0000
#define DASHBOARD_TEXT_COLOR         0xFFFF

#define DASHBOARD_CARD_VALUE_FONT    (&YandexSansDisplay_Bold12pt7b)
#define DASHBOARD_CARD_UNIT_FONT     (&YandexSansDisplay_Light12pt7b)
#define DASHBOARD_TEXT_BAND_HEIGHT   32

/*
 * Main CO2 gauge position.
 */
#define CO2_GAUGE_CENTER_X            120
#define CO2_GAUGE_CENTER_Y            115

#define CO2_GAUGE_RADIUS               95
#define CO2_GAUGE_THICKNESS             8

#define CO2_MIN_PPM                  400.0f
#define CO2_MAX_PPM                 2000.0f

#define CO2_LABEL_Y_OFFSET            (-60)
#define CO2_VALUE_Y_OFFSET            (-16)
#define CO2_UNIT_Y_OFFSET              12

#define FACE_ORIGIN_X                 120
#define FACE_ORIGIN_Y                 170

#define WARMUP_STEP                   0.01f

#define TEMP_CARD_X                    10
#define TEMP_CARD_Y                   225

#define HUMIDITY_CARD_X                85
#define HUMIDITY_CARD_Y               225

#define PRESSURE_CARD_X               160
#define PRESSURE_CARD_Y               225

#define SENSOR_CARD_WIDTH              70
#define SENSOR_CARD_HEIGHT             90

#define CARD_ICON_OFFSET_Y              8
#define CARD_VALUE_OFFSET_Y            50
#define CARD_UNIT_OFFSET_Y             70

static const char CO2_UNIT_TEXT[] = "ppm";
static const char WARMUP_TEXT[] = u8"Нагрев:)";
static const char PRESSURE_UNIT_TEXT[] = u8"мм рт.ст.";

/******************************************************************************
 * Static data
 *****************************************************************************/

static float warmupPosition = 0.0f;
static int warmupDirection = 1;

static const FaceAnimationConfig_t faceConfig =
{
    .originX = FACE_ORIGIN_X,
    .originY = FACE_ORIGIN_Y,
    .eyeColor = 0xFFFF,
    .pupilColor = 0x0000,
    .highlightColor = 0xFFFF,
    .mouthColor = 0xFFFF
};

/******************************************************************************
 * Private functions
 *****************************************************************************/

static float ScreenDashboard_GetDisplayValue(
    const SensorData_t* data
)
{
    switch(data->co2_status)
    {
    case SENSOR_CO2_STATUS_WARMUP:

        return
            CO2_MIN_PPM +
            warmupPosition *
            (
                CO2_MAX_PPM -
                CO2_MIN_PPM
            );

    case SENSOR_CO2_STATUS_OK:

        return (float)data->co2;

    default:

        return CO2_MIN_PPM;
    }
}

static const char* ScreenDashboard_GetValueText(
    const SensorData_t* data,
    char* buffer,
    uint16_t bufferSize
)
{
    switch(data->co2_status)
    {
    case SENSOR_CO2_STATUS_WARMUP:

        return WARMUP_TEXT;

    case SENSOR_CO2_STATUS_OK:

        snprintf(
            buffer,
            bufferSize,
            "%u",
            data->co2
        );

        return buffer;

    default:

        return "---";
    }
}

static void ScreenDashboard_FormatFloat1(
    float value,
    char* text
)
{
    int16_t scaled =
        (int16_t)(value * 10.0f);

    int16_t integer =
        scaled / 10;

    int16_t fraction =
        scaled % 10;

    if(fraction < 0)
    {
        fraction = -fraction;
    }

    snprintf(
        text,
        8,
        "%d.%d",
        integer,
        fraction
    );
}

static void ScreenDashboard_DrawCenteredTextBand(
    uint16_t screenY,
    int16_t bandTopY,
    uint16_t bandHeight,
    uint16_t* lineBuffer,
    uint16_t width,
    const char* text,
    const GFXfont* font,
    uint16_t color
)
{
    if(text == NULL)
    {
        return;
    }

    if(screenY >= bandTopY &&
       screenY < bandTopY + bandHeight)
    {
        const uint16_t textWidth =
            GFX_GetStringWidth(
                text,
                font
            );

        GFX_DrawStringLine(
            screenY - bandTopY,
            lineBuffer,
            width,
            text,
            (width - textWidth) / 2,
            color,
            font
        );
    }
}

static void ScreenDashboard_DrawCardValue(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const SensorCard_t* card,
    const char* text
)
{
    const int16_t valueY =
        card->y +
        CARD_VALUE_OFFSET_Y;

    const uint16_t textWidth =
        GFX_GetStringWidth(
            text,
            DASHBOARD_CARD_VALUE_FONT
        );

    const int16_t valueX =
        card->x +
        (card->width - textWidth) / 2;

    if(y >= valueY &&
       y < valueY + DASHBOARD_TEXT_BAND_HEIGHT)
    {
        GFX_DrawStringLine(
            y - valueY,
            lineBuffer,
            width,
            text,
            valueX,
            DASHBOARD_TEXT_COLOR,
            DASHBOARD_CARD_VALUE_FONT
        );
    }
}

static void ScreenDashboard_DrawCardUnit(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const SensorCard_t* card,
    const char* text
)
{
    const int16_t unitY =
        card->y +
        CARD_UNIT_OFFSET_Y;

    const uint16_t textWidth =
        GFX_GetStringWidth(
            text,
            DASHBOARD_CARD_UNIT_FONT
        );

    const int16_t unitX =
        card->x +
        (card->width - textWidth) / 2;

    if(y >= unitY &&
       y < unitY + DASHBOARD_TEXT_BAND_HEIGHT)
    {
        GFX_DrawStringLine(
            y - unitY,
            lineBuffer,
            width,
            text,
            unitX,
            DASHBOARD_TEXT_COLOR,
            DASHBOARD_CARD_UNIT_FONT
        );
    }
}

static void ScreenDashboard_DrawTempUnit(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const SensorCard_t* card
)
{
    const int16_t unitY =
        card->y +
        CARD_UNIT_OFFSET_Y;

    const char* unit = "C";
    const int16_t spacing = 1;

    const uint16_t unitWidth =
        GFX_GetStringWidth(
            unit,
            DASHBOARD_CARD_UNIT_FONT
        );

    const uint16_t groupWidth =
        BitmapDegree6.width +
        spacing +
        unitWidth;

    const int16_t groupX =
        card->x +
        (card->width - groupWidth) / 2;

    const int16_t degreeX = groupX;
    const int16_t unitX =
        groupX +
        BitmapDegree6.width +
        spacing;

    if(y >= unitY &&
       y < unitY + DASHBOARD_TEXT_BAND_HEIGHT)
    {
        GFX_DrawStringLine(
            y - unitY,
            lineBuffer,
            width,
            unit,
            unitX,
            DASHBOARD_TEXT_COLOR,
            DASHBOARD_CARD_UNIT_FONT
        );
    }

    Bitmap_RenderLine(
        y,
        lineBuffer,
        width,
        degreeX,
        unitY - 4,
        &BitmapDegree6,
        DASHBOARD_TEXT_COLOR
    );
}

static void ScreenDashboard_UpdateFace(
    const SensorData_t* data,
    const RingGauge_t* gauge
)
{
    int16_t markerX = 0;
    int16_t markerY = 0;

    RingGauge_GetMarkerPosition(
        gauge,
        &markerX,
        &markerY
    );

    const FaceAnimationInput_t faceInput =
    {
        .co2Status = data->co2_status,
        .markerX = markerX,
        .markerY = markerY
    };

    FaceAnimation_Update(
        &faceInput
    );
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

void ScreenDashboard_Init(void)
{
    FaceAnimation_Init();
}

void ScreenDashboard_Update(void)
{
    warmupPosition +=
        warmupDirection *
        WARMUP_STEP;

    if(warmupPosition >= 1.0f)
    {
        warmupPosition = 1.0f;
        warmupDirection = -1;
    }

    if(warmupPosition <= 0.0f)
    {
        warmupPosition = 0.0f;
        warmupDirection = 1;
    }

    const SensorData_t* data =
        SensorModel_Get();

    const RingGauge_t gauge =
    {
        .centerX = CO2_GAUGE_CENTER_X,
        .centerY = CO2_GAUGE_CENTER_Y,
        .radius = CO2_GAUGE_RADIUS,
        .thickness = CO2_GAUGE_THICKNESS,
        .value =
            ScreenDashboard_GetDisplayValue(
                data
            ),
        .minValue = CO2_MIN_PPM,
        .maxValue = CO2_MAX_PPM,
        .foregroundColor = 0x07E0,
        .backgroundColor = 0x39E7,
        .label = "CO2"
    };

    ScreenDashboard_UpdateFace(
        data,
        &gauge
    );
}

void ScreenDashboard_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width
)
{
    for(uint16_t i = 0; i < width; i++)
    {
        lineBuffer[i] =
            DASHBOARD_BG_COLOR;
    }

    const SensorData_t* data =
        SensorModel_Get();

    RingGauge_t co2Gauge =
    {
        .centerX = CO2_GAUGE_CENTER_X,
        .centerY = CO2_GAUGE_CENTER_Y,

        .radius = CO2_GAUGE_RADIUS,
        .thickness = CO2_GAUGE_THICKNESS,

        .value =
            ScreenDashboard_GetDisplayValue(
                data
            ),

        .minValue = CO2_MIN_PPM,
        .maxValue = CO2_MAX_PPM,

        .foregroundColor = 0x07E0,
        .backgroundColor = 0x39E7,

        .label = "CO2"
    };

    RingGauge_RenderLine(
        y,
        lineBuffer,
        width,
        &co2Gauge
    );

    const int16_t labelY =
        CO2_GAUGE_CENTER_Y +
        CO2_LABEL_Y_OFFSET;

    const int16_t valueY =
        CO2_GAUGE_CENTER_Y +
        CO2_VALUE_Y_OFFSET;

    const int16_t unitY =
        CO2_GAUGE_CENTER_Y +
        CO2_UNIT_Y_OFFSET;

    char valueBuffer[16];

    const char* valueText =
        ScreenDashboard_GetValueText(
            data,
            valueBuffer,
            sizeof(valueBuffer)
        );

    if(y >= labelY &&
       y < labelY + GFX_CO2_LABEL_BAND_HEIGHT)
    {
        GFX_DrawCo2LabelLine(
            y - labelY,
            lineBuffer,
            width,
            CO2_GAUGE_CENTER_X,
            DASHBOARD_TEXT_COLOR,
            &FreeSansBold12pt7b,
            &FreeSans7pt8b
        );
    }

    if(valueText != NULL)
    {
        const GFXfont* valueFont =
            (data->co2_status ==
             SENSOR_CO2_STATUS_WARMUP) ?
            &YandexSansDisplay_Bold20pt7b :
            &YandexSansDisplay_Bold20pt7b;

        ScreenDashboard_DrawCenteredTextBand(
            y,
            valueY,
            DASHBOARD_TEXT_BAND_HEIGHT,
            lineBuffer,
            width,
            valueText,
            valueFont,
            DASHBOARD_TEXT_COLOR
        );
    }

    if(data->co2_status ==
       SENSOR_CO2_STATUS_OK)
    {
        ScreenDashboard_DrawCenteredTextBand(
            y,
            unitY,
            DASHBOARD_TEXT_BAND_HEIGHT,
            lineBuffer,
            width,
            CO2_UNIT_TEXT,
            &FreeSans9pt7b,
            DASHBOARD_TEXT_COLOR
        );
    }

    FaceAnimation_RenderLine(
        y,
        lineBuffer,
        width,
        &faceConfig
    );

    static const SensorCard_t tempCard =
    {
        .x = TEMP_CARD_X,
        .y = TEMP_CARD_Y,
        .width = SENSOR_CARD_WIDTH,
        .height = SENSOR_CARD_HEIGHT,
        .borderColor = 0x2104,
        .backgroundColor = 0x2104
    };

    SensorCard_RenderLine(
        y,
        lineBuffer,
        width,
        &tempCard
    );

    Bitmap_RenderLine(
        y,
        lineBuffer,
        width,
        tempCard.x +
            (tempCard.width - BitmapTemp32.width) / 2,
        tempCard.y +
            CARD_ICON_OFFSET_Y,
        &BitmapTemp32,
        0xFFFF
    );

    char tempText[8];

    ScreenDashboard_FormatFloat1(
        data->temperature,
        tempText
    );

    ScreenDashboard_DrawCardValue(
        y,
        lineBuffer,
        width,
        &tempCard,
        tempText
    );

    ScreenDashboard_DrawTempUnit(
        y,
        lineBuffer,
        width,
        &tempCard
    );

    static const SensorCard_t humidityCard =
    {
        .x = HUMIDITY_CARD_X,
        .y = HUMIDITY_CARD_Y,
        .width = SENSOR_CARD_WIDTH,
        .height = SENSOR_CARD_HEIGHT,
        .borderColor = 0x2104,
        .backgroundColor = 0x2104
    };

    SensorCard_RenderLine(
        y,
        lineBuffer,
        width,
        &humidityCard
    );

    Bitmap_RenderLine(
        y,
        lineBuffer,
        width,
        humidityCard.x +
            (humidityCard.width - BitmapHumidity32.width) / 2,
        humidityCard.y +
            CARD_ICON_OFFSET_Y,
        &BitmapHumidity32,
        0xFFFF
    );

    char humidityText[8];

    ScreenDashboard_FormatFloat1(
        data->humidity,
        humidityText
    );

    ScreenDashboard_DrawCardValue(
        y,
        lineBuffer,
        width,
        &humidityCard,
        humidityText
    );

    ScreenDashboard_DrawCardUnit(
        y,
        lineBuffer,
        width,
        &humidityCard,
        "%"
    );

    static const SensorCard_t pressureCard =
    {
        .x = PRESSURE_CARD_X,
        .y = PRESSURE_CARD_Y,
        .width = SENSOR_CARD_WIDTH,
        .height = SENSOR_CARD_HEIGHT,
        .borderColor = 0x2104,
        .backgroundColor = 0x2104
    };

    SensorCard_RenderLine(
        y,
        lineBuffer,
        width,
        &pressureCard
    );

    Bitmap_RenderLine(
        y,
        lineBuffer,
        width,
        pressureCard.x +
            (pressureCard.width - BitmapPressure32.width) / 2,
        pressureCard.y +
            CARD_ICON_OFFSET_Y,
        &BitmapPressure32,
        0xFFFF
    );

    char pressureText[8];

    ScreenDashboard_FormatFloat1(
        SensorModel_GetPressureMMHg(),
        pressureText
    );

    ScreenDashboard_DrawCardValue(
        y,
        lineBuffer,
        width,
        &pressureCard,
        pressureText
    );

    ScreenDashboard_DrawCardUnit(
        y,
        lineBuffer,
        width,
        &pressureCard,
        PRESSURE_UNIT_TEXT
    );
}
