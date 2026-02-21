/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DISP_D2_Pin GPIO_PIN_2
#define DISP_D2_GPIO_Port GPIOE
#define DISP_D3_Pin GPIO_PIN_3
#define DISP_D3_GPIO_Port GPIOE
#define DISP_D4_Pin GPIO_PIN_4
#define DISP_D4_GPIO_Port GPIOE
#define DISP_D5_Pin GPIO_PIN_5
#define DISP_D5_GPIO_Port GPIOE
#define DISP_D6_Pin GPIO_PIN_6
#define DISP_D6_GPIO_Port GPIOE
#define LED_Pin GPIO_PIN_0
#define LED_GPIO_Port GPIOC
#define DISP_LED_Pin GPIO_PIN_1
#define DISP_LED_GPIO_Port GPIOC
#define DISP_D7_Pin GPIO_PIN_7
#define DISP_D7_GPIO_Port GPIOE
#define DISP_CS_Pin GPIO_PIN_8
#define DISP_CS_GPIO_Port GPIOE
#define DISP_WRX_Pin GPIO_PIN_9
#define DISP_WRX_GPIO_Port GPIOE
#define DISP_RD_Pin GPIO_PIN_10
#define DISP_RD_GPIO_Port GPIOE
#define DISP_RES_Pin GPIO_PIN_12
#define DISP_RES_GPIO_Port GPIOE
#define DISP_SCL_Pin GPIO_PIN_13
#define DISP_SCL_GPIO_Port GPIOB
#define SDIO_CD_Pin GPIO_PIN_12
#define SDIO_CD_GPIO_Port GPIOD
#define USB_VBUS_Pin GPIO_PIN_8
#define USB_VBUS_GPIO_Port GPIOA
#define DISP_D0_Pin GPIO_PIN_0
#define DISP_D0_GPIO_Port GPIOE
#define DISP_D1_Pin GPIO_PIN_1
#define DISP_D1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
