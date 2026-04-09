#include "sensor_model.h"

static SensorData_t sensorData;

void SensorModel_Set(const SensorData_t *data)
{
    sensorData = *data;
}

const SensorData_t* SensorModel_Get(void)
{
    return &sensorData;
}

float SensorModel_GetTemperature(void)
{
    return sensorData.temperature;
}

float SensorModel_GetHumidity(void)
{
    return sensorData.humidity;
}

float SensorModel_GetPressurePa(void)
{
    return sensorData.pressure;
}

float SensorModel_GetPressureMMHg(void)
{
    return sensorData.pressure * 0.750062f;
}

uint16_t SensorModel_GetCO2(void)
{
    return sensorData.co2;
}
