/******************************************************************************
 * screen_dashboard.c
 *
 * Main dashboard screen.
 *
 * Current features:
 * - CO2 circular gauge
 * - warmup animation
 * - sensor status display
 *
 * Planned:
 * - temperature widget
 * - humidity widget
 * - pressure widget
 * - animated face / emotions
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

#include "temp_32.h"
#include "icon_renderer.h"

#include "gfx_text_renderer.h"
#include "FreeSans12pt7b.h"
#include "FreeSans9pt7b.h"
#include "FreeSansBold12pt7b.h"
#include "FreeSansOblique12pt7b.h"



/******************************************************************************
 * Configuration
 *****************************************************************************/

#define DASHBOARD_BG_COLOR       0x0000
#define DASHBOARD_TEXT_COLOR     0xFFFF

/*
 * Main CO2 gauge position.
 */
#define CO2_GAUGE_CENTER_X        120
#define CO2_GAUGE_CENTER_Y        115

#define CO2_GAUGE_RADIUS           95
#define CO2_GAUGE_THICKNESS         8
/*
 * CO2 measurement range.
 */
#define CO2_MIN_PPM              400.0f
#define CO2_MAX_PPM             2000.0f

/*
 * Text placement inside gauge.
 */
#define CO2_LABEL_Y_OFFSET        (-60)
#define CO2_VALUE_Y_OFFSET        (-25)
#define CO2_UNIT_Y_OFFSET          5

/*
 * Warmup animation.
 */
#define WARMUP_STEP             0.01f

/******************************************************************************
 * Dashboard layout
 *****************************************************************************/

/*
 * Temperature card.
 */
#define TEMP_CARD_X             10
#define TEMP_CARD_Y             225

/*
 * Common card size.
 */
#define SENSOR_CARD_WIDTH       70
#define SENSOR_CARD_HEIGHT      90

/*
 * Card content layout.
 */
#define CARD_ICON_OFFSET_Y      8
#define CARD_VALUE_OFFSET_Y     40
#define CARD_UNIT_OFFSET_Y      65

/******************************************************************************
 * Temperature card layout
 *****************************************************************************/

#define TEMP_ICON_Y_OFFSET      8

#define TEMP_VALUE_Y_OFFSET     48

#define TEMP_UNIT_Y_OFFSET      60

/******************************************************************************
 * Static data
 *****************************************************************************/

static float warmupPosition = 0.0f;
static int warmupDirection = 1;

/******************************************************************************
 * Private functions
 *****************************************************************************/

/*
 * Convert sensor state into display value.
 *
 * During warmup marker moves across the scale.
 */
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

/*
 * Convert sensor state into text.
 */
static const char* ScreenDashboard_GetValueText(
    const SensorData_t* data,
    char* buffer,
    uint16_t bufferSize
)
{
    switch(data->co2_status)
    {
    case SENSOR_CO2_STATUS_WARMUP:

        return "WARM";

    case SENSOR_CO2_STATUS_OK:

        snprintf(
            buffer,
            bufferSize,
            "%u",
            data->co2
        );

        return buffer;

    case SENSOR_CO2_STATUS_NO_RESPONSE:

        return "---";

    case SENSOR_CO2_STATUS_OUT_OF_RANGE:

        return "---";

    default:

        return "---";
    }
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

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
}

/******************************************************************************
 * Format temperature without printf float support.
 *
 * Output examples:
 *      24.6
 *      -5.3
 ******************************************************************************/
static void ScreenDashboard_FormatTemperature(
    float temperature,
    char* text
)
{
    /*
     * BME280 cannot measure outside this range,
     * but limiting the value also guarantees that
     * the formatted string always fits into tempText.
     */
    if(temperature > 99.9f)
    {
        temperature = 99.9f;
    }

    if(temperature < -99.9f)
    {
        temperature = -99.9f;
    }

    int16_t value =
        (int16_t)(temperature * 10.0f);

    int16_t integer =
        value / 10;

    int16_t fraction =
        value % 10;

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
       y < labelY + 32)
    {
        uint16_t textWidth =
            GFX_GetStringWidth(
                "CO2",
                &FreeSans12pt7b
            );

        GFX_DrawStringLine(
            y - labelY,
            lineBuffer,
            width,
            "CO2",
            (width - textWidth) / 2,
            DASHBOARD_TEXT_COLOR,
            &FreeSans12pt7b
        );
    }

    if(y >= valueY &&
       y < valueY + 32)
    {
        uint16_t textWidth =
            GFX_GetStringWidth(
                valueText,
                &FreeSansBold12pt7b
            );

        GFX_DrawStringLine(
            y - valueY,
            lineBuffer,
            width,
            valueText,
            (width - textWidth) / 2,
            DASHBOARD_TEXT_COLOR,
            &FreeSansBold12pt7b
        );
    }

    if(data->co2_status ==
       SENSOR_CO2_STATUS_OK)
    {
        if(y >= unitY &&
           y < unitY + 32)
        {
            uint16_t textWidth =
                GFX_GetStringWidth(
                    "ppm",
                    &FreeSans9pt7b
                );

            GFX_DrawStringLine(
                y - unitY,
                lineBuffer,
                width,
                "ppm",
                (width - textWidth) / 2,
                DASHBOARD_TEXT_COLOR,
                &FreeSans9pt7b
            );
        }
    }

    /******************************************************************************
     * Temperature text layout
     ******************************************************************************/
    static const SensorCard_t tempCard =
        {
            .x = TEMP_CARD_X,
            .y = TEMP_CARD_Y,

            .width = SENSOR_CARD_WIDTH,
            .height = SENSOR_CARD_HEIGHT,

            .borderColor = 0x2104,
            .backgroundColor = 0x2104
        };

    const int16_t tempValueY =
        tempCard.y +
        CARD_VALUE_OFFSET_Y;

    const int16_t tempUnitY =
        tempCard.y +
        TEMP_UNIT_Y_OFFSET;

    SensorCard_RenderLine(
        y,
        lineBuffer,
        width,
        &tempCard
    );

    Icon_RenderLine(
        y,
        lineBuffer,
        width,

        tempCard.x +
            (tempCard.width - IconTemp32.width) / 2,

		tempCard.y +
		CARD_ICON_OFFSET_Y,

        &IconTemp32,

        0xFFFF
    );

    char tempText[8];

    ScreenDashboard_FormatTemperature(
        data->temperature,
        tempText
    );

    uint16_t textWidth =
        GFX_GetStringWidth(
            tempText,
            &FreeSans9pt7b
        );

    int16_t valueX =
        tempCard.x +
        (tempCard.width - textWidth) / 2;

    if(y >= tempValueY &&
       y < tempValueY + 32)
    {
        GFX_DrawStringLine(
			y - tempValueY,
            lineBuffer,
            width,

            tempText,

            valueX,

            DASHBOARD_TEXT_COLOR,

            &FreeSans9pt7b
        );
    }

    if(y >= tempUnitY &&
       y < tempUnitY + 32)
    {
        uint16_t textWidth =
            GFX_GetStringWidth(
                "°C",
                &FreeSans9pt7b
            );

        int16_t unitX =
            tempCard.x +
            (tempCard.width - textWidth) / 2;

        GFX_DrawStringLine(
            y - tempUnitY,
            lineBuffer,
            width,
            "°C",
            unitX,
            DASHBOARD_TEXT_COLOR,
            &FreeSans9pt7b
        );
    }
}
