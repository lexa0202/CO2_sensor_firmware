#ifndef __SD_MANAGER_H
#define __SD_MANAGER_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MSD_OK     0x00
#define MSD_ERROR  0x01

uint8_t BSP_SD_Init(void);

uint8_t BSP_SD_ReadBlocks(uint8_t *pData,
                          uint32_t BlockAddr,
                          uint32_t NumOfBlocks,
                          uint32_t Timeout);

uint8_t BSP_SD_WriteBlocks(uint8_t *pData,
                           uint32_t BlockAddr,
                           uint32_t NumOfBlocks,
                           uint32_t Timeout);

uint8_t BSP_SD_GetCardState(void);

void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo);

#ifdef __cplusplus
}
#endif

#endif
