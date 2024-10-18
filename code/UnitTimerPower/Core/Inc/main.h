/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct rtc_data
{
  uint8_t rtc_hour_wake;
  uint8_t rtc_min_wake;
  uint8_t rtc_sec_wake;
  uint8_t rtc_hour_wake_temp;
  uint8_t rtc_min_wake_temp;
  uint8_t rtc_sec_wake_temp;
  uint8_t rtc_hour_sleep;
  uint8_t rtc_min_sleep;
  uint8_t rtc_sec_sleep;
  uint8_t rtc_hour_sleep_temp;
  uint8_t rtc_min_sleep_temp;
  uint8_t rtc_sec_sleep_temp;
}rtc_data_t;

typedef struct
{
    float vin;
    float vbat;
    float vgrove;
    float iin;
    float ibat;
    float igrove;
} ina3221_data_t;

typedef struct
{
    int32_t vin;
    int32_t vbat;
    int32_t vgrove;
    int32_t iin;
    int32_t ibat;
    int32_t igrove;
} ina3221_send_data_t;

enum trigger_t {
  TIMER_TRIGGER = 1 << 0,
  BTN_TRIGGER = 1 << 1,
  I2C_TRIGGER = 1 << 2,
  ALL_TRIGGER = 0x07,
};

enum cycle_status_t {
  CYCLE_STOP = 0, 
  CYCLE_RUN
};
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern uint8_t i2c_address[1];

extern volatile rtc_data_t my_rtc_data;
extern ina3221_data_t ina3221_data;
extern volatile uint8_t rtc_wake_time_enable;
extern volatile uint8_t rtc_sleep_time_enable;
extern volatile uint8_t rtc_wake_ctrl_status;
extern volatile uint8_t pin_wake_ctrl_status;
extern volatile uint8_t wake_trigger;
extern volatile uint8_t sleep_trigger;
extern volatile uint8_t power_hold_status;
extern volatile uint8_t led_status;
extern volatile uint8_t sleep_flag;
extern volatile uint8_t cycle_status;
extern uint8_t excute_sleep_flag;
extern uint32_t excute_sleep_delay;
extern uint32_t display_after_sleep_time;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void flash_data_write_back(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BTN_A_Pin GPIO_PIN_0
#define BTN_A_GPIO_Port GPIOA
#define OLED_SCK_Pin GPIO_PIN_1
#define OLED_SCK_GPIO_Port GPIOA
#define OLED_MOSI_Pin GPIO_PIN_2
#define OLED_MOSI_GPIO_Port GPIOA
#define SYS_3V3_EN_Pin GPIO_PIN_3
#define SYS_3V3_EN_GPIO_Port GPIOA
#define BTN_B_Pin GPIO_PIN_4
#define BTN_B_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_6
#define OLED_DC_GPIO_Port GPIOA
#define OLED_RST_Pin GPIO_PIN_7
#define OLED_RST_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_0
#define OLED_CS_GPIO_Port GPIOB
#define BL_EN_Pin GPIO_PIN_1
#define BL_EN_GPIO_Port GPIOB
#define BAT_CHRG_Pin GPIO_PIN_4
#define BAT_CHRG_GPIO_Port GPIOB
#define PWR_HOLD_Pin GPIO_PIN_5
#define PWR_HOLD_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
