#ifndef MH_Z19C_DRIVER_H
#define MH_Z19C_DRIVER_H

#include <stdint.h>

void CO2_Init(void);

int CO2_Read(void);

void CO2_CalibrateZero(void);

int CO2_IsWarmedUp(void);

#endif
