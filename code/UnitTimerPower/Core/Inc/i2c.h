/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "i2c_ex.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define I2C_ADDRESS 0x56
#define FIRMWARE_VERSION 1
#define APPLICATION_ADDRESS     ((uint32_t)0x08001800) 
#define FLASH_DATA_SIZE 32
/* USER CODE END Private defines */

void MX_I2C1_Init(void);
void MX_I2C2_Init(void);

/* USER CODE BEGIN Prototypes */
void I2C2_Start(void);
uint8_t I2C_Write_Bytes(uint8_t DeviceAddress, uint8_t MemAddress, uint8_t *pData, uint8_t TxLen,uint16_t Timeout);
uint8_t I2C_Read_Bytes(uint8_t DeviceAddress, uint8_t MemAddress, uint8_t *pData, uint8_t RxLen,uint16_t Timeout);
void user_i2c_init(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

