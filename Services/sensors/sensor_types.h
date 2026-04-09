#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>

/* CO2 sensor status */

typedef enum
{
    SENSOR_CO2_STATUS_WARMUP,
    SENSOR_CO2_STATUS_OK,
    SENSOR_CO2_STATUS_NO_RESPONSE,
    SENSOR_CO2_STATUS_OUT_OF_RANGE

} SensorCO2_Status_t;

#endif
