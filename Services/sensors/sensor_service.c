#include "sensor_service.h"

#include "sensor_model.h"

#include "bme280_driver.h"
#include "mh-z19c_driver.h"
#include "sensor_types.h"

#include "debug_console.h" // delete if don't use debug

#include "usart.h"
#include "main.h"
#include <stdio.h>

#define BME280_UPDATE_PERIOD   1000
#define MHZ19C_UPDATE_PERIOD   5000
#define CALIBRATION_HOLD_TIME 10000

static uint32_t lastBME280 = 0;
static uint32_t lastMHZ19C = 0;
/* BME measurement state */
static uint32_t bme280_ready_time = 0;
static uint8_t bme280_measurement_running = 0;

static float temperature = 0.0f;
static float humidity = 0.0f;
static float pressure = 0.0f;

static int co2 = 0;
static SensorCO2_Status_t mhz19c_status = SENSOR_CO2_STATUS_WARMUP;
static uint32_t cal_press_start = 0;
static uint8_t cal_in_progress = 0;

void Sensor_Service_Init(void)
{
    BME280_Init();

    MHZ19C_Init();
}

void Sensor_Service_Process(void)
{
    uint32_t now = HAL_GetTick();

    /* ---- BME280 (1 раз в секунду) запуск измерения ---- */

    if(!bme280_measurement_running &&
       now - lastBME280 >= BME280_UPDATE_PERIOD)
    {
        lastBME280 = now;

        BME280_StartMeasurement();

        bme280_ready_time = now + 10;
        bme280_measurement_running = 1;
    }

    /* ---- BME280 чтение результата ---- */

    if(bme280_measurement_running &&
       now >= bme280_ready_time)
    {
        bme280_measurement_running = 0;

        BME280_ReadMeasurement(&temperature,
                               &humidity,
                               &pressure);

    }


    /* --- CO2 (1 раз в 5 секунд)--- */
        /* период измерения CO2 может зависеть от режима устройства
               сейчас режим всегда NORMAL, но архитектура готова к батарее и режиму энергосбережения */

    uint32_t mhz19c_period = MHZ19C_UPDATE_PERIOD;
    /* в будущем здесь появится:
               if(Power_GetDeviceMode() == DEVICE_MODE_BATTERY)
                   mhz19c_period = 15000;
            */

    if(now - lastMHZ19C >= mhz19c_period)
    {
        lastMHZ19C = now;

        SensorCO2_Status_t new_status;
        int new_co2 = co2;

        if(MHZ19C_GetStatus() == MHZ19C_STATUS_WARMUP)
        {
            new_status = SENSOR_CO2_STATUS_WARMUP;
        }
        else
        {
            int value = MHZ19C_ReadCO2();

            if(value < 0)
            {
                new_status = SENSOR_CO2_STATUS_NO_RESPONSE;
            }
            else if(value < 400 || value > 5000)
            {
                new_co2 = value;
                new_status = SENSOR_CO2_STATUS_OUT_OF_RANGE;
            }
            else
            {
                new_co2 = value;
                new_status = SENSOR_CO2_STATUS_OK;
            }
        }

        if(new_co2 != co2 || new_status != mhz19c_status)
        {
            co2 = new_co2;
            mhz19c_status = new_status;
        }
    }

    /* ---- обновление модели ---- */

    SensorData_t data;

    data.temperature = temperature;
    data.humidity = humidity;
    data.pressure = pressure;

    data.co2 = co2;
    data.co2_status = mhz19c_status;

    SensorModel_Set(&data);

    /* ---- CO2 calibration button ---- */

    GPIO_PinState btn = HAL_GPIO_ReadPin(BUT1_GPIO_Port, BUT1_Pin);

    if(btn == GPIO_PIN_RESET) // нажато (замкнуто на землю)
    {
        if(!cal_in_progress)
        {
            cal_press_start = now;
            cal_in_progress = 1;

            Debug_Printf("CAL: hold started\r\n");
        }
        else
        {
            uint32_t hold_time = now - cal_press_start;

            if(hold_time >= CALIBRATION_HOLD_TIME)
            {
            	if(MHZ19C_GetStatus() == MHZ19C_STATUS_OK)
            	{
            	    MHZ19C_CalibrateZero();
            	    Debug_Printf("CAL: FORCED DONE\r\n");
            	}
            	else
            	{
            	    Debug_Printf("CAL: BLOCKED (warmup)\r\n");
            	}

                cal_in_progress = 0; // сброс после выполнения
            }
        }
    }
    else
    {
        cal_in_progress = 0;
    }
}


