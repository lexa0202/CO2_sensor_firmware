#include "sd_manager.h"
#include "main.h"
#include "stm32f4xx_hal.h"

extern SD_HandleTypeDef hsd;
static bool sd_initialized = false;
static HAL_SD_CardInfoTypeDef sd_info;

/* === Внутренняя функция === */
static bool SD_EnableWideBus(void)
{
	return (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) == HAL_OK);
}

/* === Проверка наличия карты === */
bool SD_IsInserted(void)
{
    /* Предполагаем:
       LOW = карты нет
       HIGH = карта есть
    */
    return (HAL_GPIO_ReadPin(SDIO_CD_GPIO_Port, SDIO_CD_Pin) == GPIO_PIN_SET);
	//return true;
}

/* === Инициализация === */
bool SD_Init(void)
{
    if (sd_initialized)
        return true;

    if (!SD_IsInserted())
        return false;

    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;

    if (HAL_SD_Init(&hsd) != HAL_OK)
        return false;

    if (HAL_SD_GetCardInfo(&hsd, &sd_info) != HAL_OK)
        return false;

    if (!SD_EnableWideBus())
        return false;

    sd_initialized = true;
    return true;
}
/* === Деинициализация === */
void SD_DeInit(void)
{
    HAL_SD_DeInit(&hsd);
    sd_initialized = false;
}

/* === Ожидание готовности === */
bool SD_WaitReady(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
    {
        if ((HAL_GetTick() - start) > timeout_ms)
            return false;
    }

    return true;
}

/* === Чтение (multi-block) === */
bool SD_ReadBlocks(uint8_t *buf,
                   uint32_t blk_addr,
                   uint16_t blk_len)
{
	if (!sd_initialized)
	    return false;

    if (!SD_IsInserted())
        return false;

    if (blk_len == 0)
        return true;

    if (HAL_SD_ReadBlocks(&hsd,
                          buf,
                          blk_addr,
                          blk_len,
                          HAL_MAX_DELAY) != HAL_OK)
        return false;

    return SD_WaitReady(5000);
}

/* === Запись (multi-block) === */
bool SD_WriteBlocks(uint8_t *buf,
                    uint32_t blk_addr,
                    uint16_t blk_len)
{
	if (!sd_initialized)
	    return false;

    if (!SD_IsInserted())
        return false;

	if (blk_len == 0)
	    return true;

    if (HAL_SD_WriteBlocks(&hsd,
                           buf,
                           blk_addr,
                           blk_len,
                           HAL_MAX_DELAY) != HAL_OK)
        return false;

    return SD_WaitReady(5000);
}

/* === Количество блоков === */
uint32_t SD_GetBlockCount(void)
{
	if (!sd_initialized)
	    return 0;
    return sd_info.BlockNbr;
}

/* === Размер блока === */
uint32_t SD_GetBlockSize(void)
{
    if (!sd_initialized)
        return 0;
    return sd_info.BlockSize;
}
/* === Версия SD === */
uint32_t SD_GetCardVersion(void)
{
    if (!sd_initialized)
        return 0;
    return hsd.SdCard.CardVersion;
}

