/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "u8g2_disp_fun.h"
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

uint8_t led_on_sleep_timeout_flag = 0;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.SubSeconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  // sAlarm.AlarmTime.Hours = 0;
  // sAlarm.AlarmTime.Minutes = 0;
  // sAlarm.AlarmTime.Seconds = 0;
  // sAlarm.AlarmTime.SubSeconds = 0;
  // sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  // sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  // sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  // sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  // sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  // sAlarm.AlarmDateWeekDay = 1;
  // sAlarm.Alarm = RTC_ALARM_A;
  // if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  // {
  //   Error_Handler();
  // }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
  if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
  {
    FlagStatus pwrclkchanged = RESET;
    /* Update LSE configuration in Backup Domain control register */
    /* Requires to enable write access to Backup Domain if necessary */
    if (LL_APB1_GRP1_IsEnabledClock (LL_APB1_GRP1_PERIPH_PWR) != 1U)
    {
      /* Enables the PWR Clock and Enables access to the backup domain */
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
      pwrclkchanged = SET;
    }
    if (LL_PWR_IsEnabledBkUpAccess () != 1U)
    {
      /* Enable write access to Backup domain */
      LL_PWR_EnableBkUpAccess();
      while (LL_PWR_IsEnabledBkUpAccess () == 0U)
      {
      }
    }
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
    LL_RCC_LSE_Enable();

   /* Wait till LSE is ready */
    while(LL_RCC_LSE_IsReady() != 1)
    {
    }
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    /* Restore clock configuration if changed */
    if (pwrclkchanged == SET)
    {
      LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_PWR);
    }
  }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_TAMP_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(RTC_TAMP_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_TAMP_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* Function to set the current time */
void set_time_to_zero(void) 
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.SubSeconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
}

/* Function to set the Alarm time */
void set_RTC_Alarm(uint8_t hours, uint8_t minutes, uint8_t seconds) 
{
  RTC_AlarmTypeDef sAlarm = {0};
  uint8_t h=0,m=0,s=0,d=1;

	if ( seconds > 59 )
	{
		s = seconds % 60 ;
    m += (seconds / 60);
	}
  else {
    s = seconds;
  }
	if ( m + minutes > 59 )
	{
		m = (m + minutes) % 60 ;
    h += ((m + minutes) / 60);
	}
  else {
    m = m + minutes;
  }
	if ( h + hours > 23 )
	{
		h = (h + hours) % 24 ;
    d += ((h + hours) / 24);
	}
  else {
    h = h + hours;
  }  

  sAlarm.AlarmTime.Hours = h;
  sAlarm.AlarmTime.Minutes = m;
  sAlarm.AlarmTime.Seconds = s;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = d;
  sAlarm.Alarm = RTC_ALARM_A;

  set_time_to_zero();

  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
    Error_Handler();
  }
}

/* Alarm Event */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) 
{
  // Alarm event handling - Handle the wake-up event here
  if (led_on_sleep_timeout_flag) {
    rtc_sleep_time_enable = 0;
    rtc_wake_time_enable = 0;
    HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_SET);
    power_hold_status = 1;      
    sleep_flag = 0;
    led_on_sleep_timeout_flag = 0;
  }
  if (rtc_wake_time_enable) {
    if (!oled_on_mode) {
      if (((wake_trigger & TIMER_TRIGGER) && (my_rtc_data.rtc_hour_sleep || my_rtc_data.rtc_min_sleep || my_rtc_data.rtc_sec_sleep)) || (wake_trigger & BTN_TRIGGER)) {
        HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
        power_hold_status = 0;
        if (!excute_sleep_flag) {
          if (wake_trigger & TIMER_TRIGGER) {
            __HAL_RCC_PWR_CLK_ENABLE();
            // HAL_PWREx_EnableInternalWakeUpLine();          
            set_RTC_Alarm(my_rtc_data.rtc_hour_sleep, my_rtc_data.rtc_min_sleep, my_rtc_data.rtc_sec_sleep);
            int32_t total_sleep_secs = my_rtc_data.rtc_hour_sleep * 3600 + my_rtc_data.rtc_min_sleep * 60 + my_rtc_data.rtc_sec_sleep;
            if (total_sleep_secs > 3)
              display_after_sleep_time = 3000;
            else if (total_sleep_secs > 0)
              display_after_sleep_time = (total_sleep_secs - 1) * 1000;
            else
              display_after_sleep_time = 0;            
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUFI);
          }
          // if (wake_trigger & BTN_TRIGGER) {
          //   HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
          //   HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);
          //   // __HAL_RCC_PWR_CLK_ENABLE();
          //   // __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUFI); 
          //   // if (!rtc_wake_ctrl_status)         
          //   //   RTC_SetAlarmFrec(0, 0, 0);
          //   // HAL_PWREx_EnableInternalWakeUpLine();          
          //   __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
          //   __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
          // }
          // else {
          //   HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
          //   HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
          // }              
          excute_sleep_delay = HAL_GetTick();
          excute_sleep_flag = 1; 
        } 
      }
      else {
        rtc_wake_time_enable = 0;
        sleep_flag = 0;
      } 
    }
    else {
      if (my_rtc_data.rtc_hour_sleep || my_rtc_data.rtc_min_sleep || my_rtc_data.rtc_sec_sleep) {             
        if (!rtc_sleep_time_enable) {
          if (my_rtc_data.rtc_hour_sleep || my_rtc_data.rtc_min_sleep || my_rtc_data.rtc_sec_sleep) {             
            set_RTC_Alarm(my_rtc_data.rtc_hour_sleep, my_rtc_data.rtc_min_sleep, my_rtc_data.rtc_sec_sleep);
            HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
            power_hold_status = 0;  
            rtc_sleep_time_enable = 1; 
            led_on_sleep_timeout_flag = 1;				
          }
        } 				
      }
      else {
        rtc_wake_time_enable = 0;
        sleep_flag = 0;
      }              
    }
  }
}
/* USER CODE END 1 */
