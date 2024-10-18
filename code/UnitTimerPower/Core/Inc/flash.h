/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flash.h
  * @brief          : flash function
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
#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32g0xx.h"
#include <stdbool.h>
#include "stm32g0xx_hal_flash_ex.h"

//Message head
#define EEPPROM_PACKAGEHEAD 0xAA55//

//Flash page head
#define STM32G0xx_PAGE_SIZE 0x800
#define STM32G0xx_FLASH_PAGE0_STARTADDR 0x8000000
#define STM32G0xx_FLASH_PAGE1_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE2_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+2*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE3_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+3*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE4_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+4*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE5_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+5*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE6_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+6*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE7_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+7*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE8_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+8*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE9_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+9*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE10_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+10*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE11_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+11*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE12_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+12*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE13_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+13*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE14_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+14*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE15_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+15*STM32G0xx_PAGE_SIZE)
#define STM32G0xx_FLASH_PAGE31_STARTADDR (STM32G0xx_FLASH_PAGE0_STARTADDR+31*STM32G0xx_PAGE_SIZE)


#define MIN(A,B) (A<B?A:B)


// void flashReadWriteTest( void ) ;
bool writeMessageToFlash( uint8_t *buff , uint16_t length);
uint16_t readPackedMessageFromFlash( uint8_t *buff , uint16_t length);

#endif
