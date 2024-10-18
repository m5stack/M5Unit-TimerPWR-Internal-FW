/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include <string.h>
/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* I2C1 interrupt Init */
  NVIC_SetPriority(I2C1_IRQn, 0);
  NVIC_EnableIRQ(I2C1_IRQn);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x00303D5B;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0x56<<1;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}
/* I2C2 init function */
void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**I2C2 GPIO Configuration
  PA11 [PA9]   ------> I2C2_SCL
  PA12 [PA10]   ------> I2C2_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */

  /** I2C Initialization
  */
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x0010061A;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C2, &I2C_InitStruct);
  LL_I2C_EnableAutoEndMode(I2C2);
  LL_I2C_SetOwnAddress2(I2C2, 0, LL_I2C_OWNADDRESS2_NOMASK);
  LL_I2C_DisableOwnAddress2(I2C2);
  LL_I2C_DisableGeneralCall(I2C2);
  LL_I2C_EnableClockStretching(I2C2);
  /* USER CODE BEGIN I2C2_Init 2 */
  I2C2_Start();
  /* USER CODE END I2C2_Init 2 */

}

/* USER CODE BEGIN 1 */
void I2C2_Start(void)
{
  LL_I2C_Enable(I2C2);
}

void I2C2_ClearCR(void)
{
  (I2C2->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN)));
}

uint8_t I2C2_TransmitData(uint8_t device_id ,uint8_t *pdata, uint8_t size, uint16_t timeout)
{
  // busyフラグをチェック
  while(LL_I2C_IsActiveFlag_BUSY(I2C2) == SET) {
    if(LL_SYSTICK_IsActiveCounterFlag())
    {
        if (timeout-- == 0)
        {
            I2C2_ClearCR();
            return 1;
        }
    }       
  }

  LL_I2C_HandleTransfer(I2C2, device_id, LL_I2C_ADDRSLAVE_7BIT, size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

  for(uint8_t i = 0; i < size; i++){
    while(LL_I2C_IsActiveFlag_TXE(I2C2)== RESET) {
        if(LL_SYSTICK_IsActiveCounterFlag())
        {
            if (timeout-- == 0)
            {
                I2C2_ClearCR();
                return 1;
            }
        }         
    }
    LL_I2C_TransmitData8(I2C2, *pdata++);
  }

  while(LL_I2C_IsActiveFlag_STOP(I2C2)==RESET) {
    if(LL_SYSTICK_IsActiveCounterFlag())
    {
        if (timeout-- == 0)
        {
            I2C2_ClearCR();
            return 1;
        }
    }       
  }
  LL_I2C_ClearFlag_STOP(I2C2);
  
  // 設定をリセットする
  I2C2_ClearCR();

  return 0;
}

uint8_t I2C2_TransmitData_RepeatedStart(uint8_t device_id ,uint8_t *pdata, uint8_t size, uint16_t timeout)
{
  // busyフラグをチェック
  while(LL_I2C_IsActiveFlag_BUSY(I2C2) == SET) {
    if(LL_SYSTICK_IsActiveCounterFlag())
    {
        if (timeout-- == 0)
        {
            I2C2_ClearCR();
            return 1;
        }
    }
  }

  LL_I2C_HandleTransfer(I2C2, device_id, LL_I2C_ADDRSLAVE_7BIT, size, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

  for(uint8_t i = 0; i < size; i++){
    while(LL_I2C_IsActiveFlag_TXE(I2C2)== RESET) {
        if(LL_SYSTICK_IsActiveCounterFlag())
        {
            if (timeout-- == 0)
            {
                I2C2_ClearCR();
                return 1;
            }
        }        
    }
    LL_I2C_TransmitData8(I2C2, *pdata++);
  }
  while(LL_I2C_IsActiveFlag_TC(I2C2)==RESET) {
    if(LL_SYSTICK_IsActiveCounterFlag())
    {
        if (timeout-- == 0)
        {
            I2C2_ClearCR();
            return 1;
        }
    }
  }
  
  return 0;
}

uint8_t I2C2_ReceiveData(uint8_t device_id , uint8_t *pdata, uint8_t size, uint8_t timeout)
{
  // 初期設定 
  LL_I2C_HandleTransfer(I2C2, device_id, LL_I2C_ADDRSLAVE_7BIT, size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

  for(uint8_t i = 0; i < size; i++){
    while(LL_I2C_IsActiveFlag_RXNE(I2C2) == RESET) {
        if(LL_SYSTICK_IsActiveCounterFlag())
        {
            if (timeout-- == 0)
            {
                I2C2_ClearCR();
                return 1;
            }
        }        
    }
    *pdata++ = LL_I2C_ReceiveData8(I2C2);
  }
  
  while(LL_I2C_IsActiveFlag_STOP(I2C2)==RESET) {
    if(LL_SYSTICK_IsActiveCounterFlag())
    {
        if (timeout-- == 0)
        {
            I2C2_ClearCR();
            return 1;
        }
    }      
  }
  LL_I2C_ClearFlag_STOP(I2C2);
  
  // 設定をリセットする
  I2C2_ClearCR();

  return 0;
}

uint8_t I2C_Write_Bytes(uint8_t DeviceAddress, uint8_t MemAddress, uint8_t *pData, uint8_t TxLen,uint16_t Timeout)
{
    uint8_t tx_buffer[TxLen+1];

    tx_buffer[0] = MemAddress;
    memcpy(&tx_buffer[1], pData, TxLen);

    return I2C2_TransmitData(DeviceAddress, tx_buffer, TxLen+1, Timeout);	
}

uint8_t I2C_Read_Bytes(uint8_t DeviceAddress, uint8_t MemAddress, uint8_t *pData, uint8_t RxLen,uint16_t Timeout)
{
    if (!I2C2_TransmitData_RepeatedStart(DeviceAddress, &MemAddress, 1, Timeout)) {
        return I2C2_ReceiveData(DeviceAddress ,pData, RxLen, Timeout);

    }
    else {
        return 1;
    }
}

void user_i2c_init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* I2C1 interrupt Init */
  NVIC_SetPriority(I2C1_IRQn, 0);
  NVIC_EnableIRQ(I2C1_IRQn);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x00303D5B;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = i2c_address[0]<<1;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */
  set_i2c_slave_address(i2c_address[0]);
  /* USER CODE END I2C1_Init 2 */

}
/* USER CODE END 1 */
