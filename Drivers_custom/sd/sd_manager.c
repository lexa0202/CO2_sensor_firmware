#include "sd_manager.h"
#include "main.h"

extern SD_HandleTypeDef hsd;

/* ==== INIT ==== */
uint8_t BSP_SD_Init(void)
{
    if (HAL_SD_Init(&hsd) != HAL_OK)
        return MSD_ERROR;

    if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
        return MSD_ERROR;

    return MSD_OK;
}

/* ==== READ ==== */
uint8_t BSP_SD_ReadBlocks(uint8_t *pData,
                          uint32_t BlockAddr,
                          uint32_t NumOfBlocks,
                          uint32_t Timeout)
{
    if (HAL_SD_ReadBlocks(&hsd,
                          pData,
                          BlockAddr,
                          NumOfBlocks,
                          Timeout) != HAL_OK)
        return MSD_ERROR;

    while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
    {
    }

    return MSD_OK;
}

/* ==== WRITE ==== */
uint8_t BSP_SD_WriteBlocks(uint8_t *pData,
                           uint32_t BlockAddr,
                           uint32_t NumOfBlocks,
                           uint32_t Timeout)
{
    if (HAL_SD_WriteBlocks(&hsd,
                           pData,
                           BlockAddr,
                           NumOfBlocks,
                           Timeout) != HAL_OK)
        return MSD_ERROR;

    while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
    {
    }

    return MSD_OK;
}

/* ==== STATE ==== */
uint8_t BSP_SD_GetCardState(void)
{
    if (HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER)
        return MSD_OK;

    return MSD_ERROR;
}

/* ==== INFO ==== */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
    HAL_SD_GetCardInfo(&hsd, CardInfo);
}
