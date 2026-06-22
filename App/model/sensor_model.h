#ifndef SENSOR_MODEL_H
#define SENSOR_MODEL_H

#include <stdint.h>

typedef enum
{
    SENSOR_CO2_STATUS_WARMUP,
    SENSOR_CO2_STATUS_OK,
    SENSOR_CO2_STATUS_NO_RESPONSE,
    SENSOR_CO2_STATUS_OUT_OF_RANGE

} SensorCO2_Status_t;

typedef struct
{
    float temperature;
    float humidity;
    float pressure;

    uint16_t co2;
    SensorCO2_Status_t co2_status;

} SensorData_t;

void SensorModel_Set(const SensorData_t *data);
const SensorData_t* SensorModel_Get(void);
float SensorModel_GetPressureMMHg(void);
float SensorModel_GetTemperature(void);
float SensorModel_GetHumidity(void);
uint16_t SensorModel_GetCO2(void);

#endif
