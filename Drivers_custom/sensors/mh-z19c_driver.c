#include "mh-z19c_driver.h"
#include "usart.h"

#include "stm32f4xx_hal.h"

#include <string.h>
#include <stdio.h>

/* --- команды датчика --- */

static const uint8_t cmd_read[9] =
{
    0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79
};

static const uint8_t cmd_zero[9] =
{
    0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78
};

static uint8_t rx_buf[9];

static uint32_t warmup_start = 0;

/* --- checksum --- */

static uint8_t checksum(uint8_t *buf)
{
    uint8_t sum = 0;

    for(int i = 1; i < 8; i++)
        sum += buf[i];

    sum = 0xFF - sum;
    sum += 1;

    return sum;
}

/* --- очистка RX --- */

static void uart_clear_rx(void)
{
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE))
    {
        volatile uint8_t dummy = huart1.Instance->DR;
        (void)dummy;
    }
}

/* --- init --- */

void MHZ19C_Init(void)
{
    warmup_start = HAL_GetTick();

    /* датчик после включения игнорирует команды ~2 сек */
    HAL_Delay(2000);
}

/* --- прогрев датчика --- */

int MHZ19C_IsWarmedUp(void)
{
    if(HAL_GetTick() - warmup_start > 180000)
        return 1;

    return 0;
}

/* --- чтение CO2 --- */

int MHZ19C_ReadCO2(void)
{
    uart_clear_rx();

    memset(rx_buf,0,sizeof(rx_buf));

    /* отправляем команду */
    HAL_UART_Transmit(&huart1,(uint8_t*)cmd_read,9,100);

    /* читаем ответ по одному байту */
    for(int i=0;i<9;i++)
    {
        if(HAL_UART_Receive(&huart1,&rx_buf[i],1,100) != HAL_OK)
        {
            return -1;
        }
    }

    /* проверка заголовка */
    if(rx_buf[0] != 0xFF)
        return -1;

    if(rx_buf[1] != 0x86)
        return -1;

    /* проверка checksum */
    if(checksum(rx_buf) != rx_buf[8])
        return -1;

    int co2 = (rx_buf[2] << 8) | rx_buf[3];

    return co2;
}

/* --- калибровка нуля --- */

void MHZ19C_CalibrateZero(void)
{
    uart_clear_rx();

    HAL_UART_Transmit(&huart1,(uint8_t*)cmd_zero,9,100);
}

MHZ19C_Status_t MHZ19C_GetStatus(void)
{
    if(!MHZ19C_IsWarmedUp())
        return MHZ19C_STATUS_WARMUP;

    return MHZ19C_STATUS_OK;
}
