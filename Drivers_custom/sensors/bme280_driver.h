#ifndef BME280_DRIVER_H
#define BME280_DRIVER_H

#include <stdint.h>

int BME280_Init(void);

int BME280_StartMeasurement(void);

int BME280_ReadMeasurement(float *temperature,
                           float *humidity,
                           float *pressure);

#endif
