#include "sensor_service.h"

#include "sensor_model.h"

#include "bme280_driver.h"
#include "mh-z19c_driver.h"

#include "debug_console.h" // delete if don't use debug

#include "usart.h"

#include <stdio.h>
// сейчас перевод в единицы измерения происходит тут, но потом нужно перенсти в sensor_model
#define BME_UPDATE_PERIOD   1000
#define CO2_UPDATE_PERIOD   5000

static uint32_t lastBME = 0;
static uint32_t lastCO2 = 0;
/* BME measurement state */
static uint32_t bme_ready_time = 0;
static uint8_t bme_measurement_running = 0;

static float temperature = 0.0f;
static float humidity = 0.0f;
static float pressure = 0.0f;

static int co2 = 0;
static CO2_Status_t co2_status = CO2_STATUS_WARMUP;


void Sensor_Service_Init(void)
{
    BME280_Init();

    CO2_Init();
}

void Sensor_Service_Process(void)
{
    uint32_t now = HAL_GetTick();

    bool updated = false;

    /* ---- BME280 (1 раз в секунду) запуск измерения ---- */

    if(!bme_measurement_running &&
       now - lastBME >= BME_UPDATE_PERIOD)
    {
        lastBME = now;

        BME280_StartMeasurement();

        bme_ready_time = now + 10;
        bme_measurement_running = 1;
    }

    /* ---- BME280 чтение результата ---- */

    if(bme_measurement_running &&
       now >= bme_ready_time)
    {
        bme_measurement_running = 0;

        BME280_ReadMeasurement(&temperature,
                               &humidity,
                               &pressure);

        updated = true;
    }


    /* --- CO2 (1 раз в 5 секунд)--- */
        /* период измерения CO2 может зависеть от режима устройства
               сейчас режим всегда NORMAL, но архитектура готова к батарее и режиму энергосбережения */

    uint32_t co2_period = CO2_UPDATE_PERIOD;
    /* в будущем здесь появится:
               if(Power_GetDeviceMode() == DEVICE_MODE_BATTERY)
                   co2_period = 15000;
            */

    if(now - lastCO2 >= co2_period)
    {
        lastCO2 = now;

        if(!CO2_IsWarmedUp())
        {
            co2_status = CO2_STATUS_WARMUP;
        }
        else
        {
            int value = CO2_Read();

            if(value > 0)
            {
                co2 = value;
                co2_status = CO2_STATUS_OK;
                updated = true;
            }
            else
            {
                co2_status = CO2_STATUS_NO_RESPONSE;
            }
        }
    }

    /* ---- обновление модели ---- */

    if(updated)
    {
        SensorData_t data;

        data.temperature = temperature;
        data.humidity = humidity;
        data.pressure = pressure;
        data.co2 = co2;

        SensorModel_Set(&data);

        float p_mmhg = pressure * 0.750062f;

        int t_i = (int)temperature;
        int t_f = (int)((temperature - t_i) * 100);

        int h_i = (int)humidity;
        int h_f = (int)((humidity - h_i) * 100);

        int p_i = (int)p_mmhg;
        int p_f = (int)((p_mmhg - p_i) * 100);

        if(co2_status == CO2_STATUS_OK)
        {
            Debug_Printf(
                "T=%d.%02dC H=%d.%02d%% P=%d.%02dmmHg CO2=%dppm\r\n",
                t_i, t_f,
                h_i, h_f,
                p_i, p_f,
                co2
            );
        }
        else if(co2_status == CO2_STATUS_WARMUP)
        {
            Debug_Printf(
                "T=%d.%02dC H=%d.%02d%% P=%d.%02dmmHg CO2=W\r\n",
                t_i, t_f,
                h_i, h_f,
                p_i, p_f
            );
        }
        else
        {
            Debug_Printf(
                "T=%d.%02dC H=%d.%02d%% P=%d.%02dmmHg CO2=N/A\r\n",
                t_i, t_f,
                h_i, h_f,
                p_i, p_f
            );
        }
    }
}


