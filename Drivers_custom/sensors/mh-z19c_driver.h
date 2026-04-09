#ifndef MH_Z19C_DRIVER_H
#define MH_Z19C_DRIVER_H

#include <stdint.h>

typedef enum
{
    MHZ19C_STATUS_WARMUP,
    MHZ19C_STATUS_OK,
    MHZ19C_STATUS_NO_RESPONSE,
    MHZ19C_STATUS_OUT_OF_RANGE
} MHZ19C_Status_t;

void MHZ19C_Init(void);

int MHZ19C_ReadCO2(void);

void MHZ19C_CalibrateZero(void);

MHZ19C_Status_t MHZ19C_GetStatus(void);

#endif
