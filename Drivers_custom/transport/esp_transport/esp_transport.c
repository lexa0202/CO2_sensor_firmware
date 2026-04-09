#include "esp_transport.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "gpio.h"


/* ===== DMA RX BUFFER ===== */
#define ESP_RX_DMA_SIZE 256
static uint8_t dma_rx_buf[ESP_RX_DMA_SIZE];
static uint16_t dma_last_pos = 0;

/* ===== RING BUFFER ===== */

#define ESP_RING_SIZE 512

static uint8_t ring_buf[ESP_RING_SIZE];
static uint16_t ring_head = 0;
static uint16_t ring_tail = 0;

/* ===== INIT ===== */

void ESP_Transport_Init(void)
{
    HAL_UART_Receive_DMA(&huart2, dma_rx_buf, ESP_RX_DMA_SIZE);
}

void ESP_Transport_PowerOn(void)
{
    // PMOS: LOW = включить питание ESP
    HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_RESET);
}

void ESP_Transport_PowerOff(void)
{
    HAL_GPIO_WritePin(ESP_EN_GPIO_Port, ESP_EN_Pin, GPIO_PIN_SET);
}
/* ===== INTERNAL: PUSH BYTE ===== */

static void ring_push(uint8_t b)
{
    uint16_t next = (ring_head + 1) % ESP_RING_SIZE;

    if(next != ring_tail) // защита от overflow
    {
        ring_buf[ring_head] = b;
        ring_head = next;
    }
}

/* ===== PROCESS DMA ===== */

void ESP_Transport_Process(void)
{
    uint16_t pos = ESP_RX_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

    if(pos != dma_last_pos)
    {
        if(pos > dma_last_pos)
        {
            for(uint16_t i = dma_last_pos; i < pos; i++)
                ring_push(dma_rx_buf[i]);
        }
        else
        {
            for(uint16_t i = dma_last_pos; i < ESP_RX_DMA_SIZE; i++)
                ring_push(dma_rx_buf[i]);

            for(uint16_t i = 0; i < pos; i++)
                ring_push(dma_rx_buf[i]);
        }

        dma_last_pos = pos;
    }
}

/* ===== SEND ===== */

void ESP_Transport_Send(const char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 100);
}

/* ===== GET LINE ===== */

bool ESP_Transport_GetLine(char *out, uint16_t maxLen)
{
    static char temp[256];
    static uint16_t temp_len = 0;

    while(ring_tail != ring_head)
    {
        char c = ring_buf[ring_tail];
        ring_tail = (ring_tail + 1) % ESP_RING_SIZE;

        if(c == '\r')
            continue;

        if(c == '\n')
        {
            temp[temp_len] = 0;

            strncpy(out, temp, maxLen);
            temp_len = 0;

            return true;
        }

        if(temp_len < sizeof(temp) - 1)
        {
            temp[temp_len++] = c;
        }
    }

    return false;
}
