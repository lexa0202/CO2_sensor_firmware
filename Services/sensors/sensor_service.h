#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

void Sensor_Service_Init(void);
void Sensor_Service_Process(void);

typedef enum
{
    CO2_STATUS_WARMUP = 0,
    CO2_STATUS_OK,
    CO2_STATUS_NO_RESPONSE
} CO2_Status_t;

#endif
