#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* Инициализация SD */
bool SD_Init(void);

/* Деинициализация SD */
void SD_DeInit(void);

/* Проверка наличия карты */
bool SD_IsInserted(void);

/* Ожидание готовности карты */
bool SD_WaitReady(uint32_t timeout_ms);

/* Чтение блоков */
bool SD_ReadBlocks(uint8_t *buf,
                   uint32_t blk_addr,
                   uint16_t blk_len);

/* Запись блоков */
bool SD_WriteBlocks(uint8_t *buf,
                    uint32_t blk_addr,
                    uint16_t blk_len);

/* Получить количество блоков */
uint32_t SD_GetBlockCount(void);

/* Размер блока (обычно 512) */
uint32_t SD_GetBlockSize(void);

uint32_t SD_GetCardVersion(void);
#endif
