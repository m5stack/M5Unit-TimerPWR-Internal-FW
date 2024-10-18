/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ina3221.h"
#include "flash.h"
#include "oled_u8g2.h"
#include "u8g2_disp_fun.h"
#include "button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define INA3221_SAMPLE_TIME 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
unsigned long ina3221_delay_time = 0;
ina3221_data_t ina3221_data;
ina3221_send_data_t ina3221_send_data;

// 定义低通滤波器参数
float alpha = 0.909f;  // 低通滤波器系数/截止频率2Hz/采样频率10Hz
float prevFilteredValue_vin = 0;  // 上一个滤波后的数值
float prevFilteredValue_iin = 0;  // 上一个滤波后的数值
float prevFilteredValue_vbat = 0;  // 上一个滤波后的数值
float prevFilteredValue_ibat = 0;  // 上一个滤波后的数值
float prevFilteredValue_vgrove = 0;  // 上一个滤波后的数值
float prevFilteredValue_igrove = 0;  // 上一个滤波后的数值

uint8_t i2c_address[1] = {0};
uint8_t flash_data[FLASH_DATA_SIZE] = {0};
volatile uint8_t power_hold_status = 0;
volatile uint8_t led_status = 1;
volatile uint8_t rtc_wake_ctrl_status = 0;
volatile uint8_t pin_wake_ctrl_status = 0;
volatile uint8_t wake_trigger = (BTN_TRIGGER | TIMER_TRIGGER);
volatile uint8_t sleep_trigger = ALL_TRIGGER;
volatile uint8_t cycle_status = CYCLE_STOP;
volatile uint8_t rtc_wake_time_enable = 0;
volatile uint8_t rtc_sleep_time_enable = 0;
volatile uint8_t sleep_flag = 0;
volatile uint8_t fm_version = FIRMWARE_VERSION;

volatile rtc_data_t my_rtc_data;

volatile uint8_t flag_jump_bootloader = 0;
volatile uint32_t jump_bootloader_timeout = 0;
volatile uint32_t i2c_stop_timeout_delay = 0;

uint8_t excute_sleep_flag = 0;
uint32_t excute_sleep_delay = 0;
uint32_t display_after_sleep_time = 0;

uint8_t iap_switch = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void IAP_Set()
{
	uint8_t i;
 
	uint32_t *pVecTab=(uint32_t *)(0x20000000);

	for(i = 0; i < 48; i++)
	{
		*(pVecTab++) = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
	}
  /* Enable the SYSCFG peripheral clock*/
#if 1 //STM32
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  __HAL_SYSCFG_REMAPMEMORY_SRAM();
#else //AMP32
    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_SYSCFG);
    /* Remap SRAM at 0x00000000 */
    SYSCFG->CFG1_B.MMSEL = SYSCFG_MemoryRemap_SRAM;
#endif
}

void i2c_port_set_to_input(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = (GPIO_PIN_6 | GPIO_PIN_7);
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void init_flash_data(void) 
{   
  if (!(readPackedMessageFromFlash(flash_data, FLASH_DATA_SIZE))) {
    i2c_address[0] = I2C_ADDRESS;

    flash_data[0] = i2c_address[0];
    flash_data[1] = cycle_status;
    flash_data[2] = my_rtc_data.rtc_hour_wake;
    flash_data[3] = my_rtc_data.rtc_min_wake;
    flash_data[4] = my_rtc_data.rtc_sec_wake;
    flash_data[5] = my_rtc_data.rtc_hour_sleep;
    flash_data[6] = my_rtc_data.rtc_min_sleep;
    flash_data[7] = my_rtc_data.rtc_sec_sleep;
    flash_data[8] = wake_trigger;
    flash_data[9] = sleep_trigger;
    flash_data[10] = work_order;
    flash_data[11] = oled_on_mode;
    flash_data[12] = led_status;
    flash_data[13] = iap_switch;
    writeMessageToFlash(flash_data , FLASH_DATA_SIZE);
  } else {
    i2c_address[0] = flash_data[0];
    cycle_status = flash_data[1];
    my_rtc_data.rtc_hour_wake = flash_data[2];
    my_rtc_data.rtc_min_wake = flash_data[3];
    my_rtc_data.rtc_sec_wake = flash_data[4];
    my_rtc_data.rtc_hour_sleep = flash_data[5];
    my_rtc_data.rtc_min_sleep = flash_data[6];
    my_rtc_data.rtc_sec_sleep = flash_data[7];   
    wake_trigger = flash_data[8];
    sleep_trigger = flash_data[9];     
    work_order = flash_data[10];
    oled_on_mode = flash_data[11];
    led_status = flash_data[12];
    iap_switch = flash_data[13];
  }
}

void flash_data_write_back(void)
{
  if (readPackedMessageFromFlash(flash_data, FLASH_DATA_SIZE)) {
    flash_data[0] = i2c_address[0];
    flash_data[1] = cycle_status;
    flash_data[2] = my_rtc_data.rtc_hour_wake;
    flash_data[3] = my_rtc_data.rtc_min_wake;
    flash_data[4] = my_rtc_data.rtc_sec_wake;
    flash_data[5] = my_rtc_data.rtc_hour_sleep;
    flash_data[6] = my_rtc_data.rtc_min_sleep;
    flash_data[7] = my_rtc_data.rtc_sec_sleep;
    flash_data[8] = wake_trigger;
    flash_data[9] = sleep_trigger;
    flash_data[10] = work_order;
    flash_data[11] = oled_on_mode;
    flash_data[12] = led_status;
    flash_data[13] = iap_switch;
    writeMessageToFlash(flash_data , FLASH_DATA_SIZE);
  }     
}

void clear_pwr_flag(void)
{
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    /* Check and Clear the Wakeup flag */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF1) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
    }
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF2) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
    }
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUFI) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUFI);
    }
  }  
}

// 低通滤波器函数
float lowPassFilter(float inputValue, uint8_t filter_ch) {
  float filteredValue;

  switch (filter_ch)
  {
  case 0:
    filteredValue = alpha * inputValue + (1 - alpha) * prevFilteredValue_vin;
    prevFilteredValue_vin = filteredValue;
    break;
  case 1:
    filteredValue = alpha * inputValue + (1 - alpha) * prevFilteredValue_vbat;
    prevFilteredValue_vbat = filteredValue;
    break;
  case 2:
    filteredValue = alpha * inputValue + (1 - alpha) * prevFilteredValue_vgrove;
    prevFilteredValue_vgrove = filteredValue;
    break;
  case 3:
    filteredValue = alpha * inputValue + (1 - alpha) * prevFilteredValue_iin;
    prevFilteredValue_iin = filteredValue;
    break;
  case 4:
    filteredValue = alpha * inputValue + (1 - alpha) * prevFilteredValue_ibat;
    prevFilteredValue_ibat = filteredValue;
    break;
  case 5:
    filteredValue = alpha * inputValue + (1 - alpha) * prevFilteredValue_igrove;
    prevFilteredValue_igrove = filteredValue;
    break;
  
  default:
    break;
  }
    
  return filteredValue;
}

void Slave_Complete_Callback(uint8_t *rx_data, uint16_t len)
{
  uint8_t buf[32];
  uint8_t rx_buf[16];
  uint8_t rx_mark[16] = {0}; 

  if (len > 1) {
    if (rx_data[0] == 0x00) {
      if (rx_data[1]) {
        HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_SET);
        power_hold_status = 1;
      } else {
        HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
        power_hold_status = 0;
      }
    }
    else if (rx_data[0] == 0x01) {
      if (rx_data[1]) {
        HAL_GPIO_WritePin(BL_EN_GPIO_Port, BL_EN_Pin, GPIO_PIN_SET);
        led_status = 1;
      } else {
        HAL_GPIO_WritePin(BL_EN_GPIO_Port, BL_EN_Pin, GPIO_PIN_RESET);
        led_status = 0;
      }
    }
    else if ((rx_data[0] >= 0x20) && (rx_data[0] <= 0x21)) {
      for(int i = 0; i < len - 1; i++) {
        rx_buf[rx_data[0]-0x20+i] = rx_data[1+i];
        rx_mark[rx_data[0]-0x20+i] = 1;     
      }
      if (rx_mark[0]) {
        wake_trigger = rx_buf[0] & 0x07;
        if (rx_buf[0]) {
          rtc_wake_ctrl_status = 1;
        } else {
          rtc_wake_ctrl_status = 0;
        }
      }      
      if (rx_mark[1]) {
        sleep_trigger = rx_buf[1] & 0x07;
        if (rx_buf[1]) {
          pin_wake_ctrl_status = 1;
        } else {
          pin_wake_ctrl_status = 0;
        }
      }      
    }
    else if ((rx_data[0] >= 0x30) && (rx_data[0] <= 0x32)) {
      for(int i = 0; i < len - 1; i++) {
        rx_buf[rx_data[0]-0x30+i] = rx_data[1+i];
        rx_mark[rx_data[0]-0x30+i] = 1;     
      }
      if (rx_mark[0]) {
        my_rtc_data.rtc_hour_wake = rx_buf[0];
      }      
      if (rx_mark[1]) {
        my_rtc_data.rtc_min_wake = rx_buf[1];
      }      
      if (rx_mark[2]) {
        my_rtc_data.rtc_sec_wake = rx_buf[2];
      } 
      my_rtc_data.rtc_hour_wake_temp = my_rtc_data.rtc_hour_wake;
      my_rtc_data.rtc_min_wake_temp = my_rtc_data.rtc_min_wake;
      my_rtc_data.rtc_sec_wake_temp = my_rtc_data.rtc_sec_wake;
      num_to_disp_num();
    }
    else if ((rx_data[0] >= 0x80) && (rx_data[0] <= 0x82)) {
      for(int i = 0; i < len - 1; i++) {
        rx_buf[rx_data[0]-0x80+i] = rx_data[1+i];
        rx_mark[rx_data[0]-0x80+i] = 1;     
      }
      if (rx_mark[0]) {
        my_rtc_data.rtc_hour_sleep = rx_buf[0];
      }      
      if (rx_mark[1]) {
        my_rtc_data.rtc_min_sleep = rx_buf[1];
      }      
      if (rx_mark[2]) {
        my_rtc_data.rtc_sec_sleep = rx_buf[2];
      }
      my_rtc_data.rtc_hour_sleep_temp = my_rtc_data.rtc_hour_sleep;
      my_rtc_data.rtc_min_sleep_temp = my_rtc_data.rtc_min_sleep;
      my_rtc_data.rtc_sec_sleep_temp = my_rtc_data.rtc_sec_sleep;  
      num_to_disp_num();            
    }
    else if (rx_data[0] == 0x40) {
      if (rx_data[1]) {
        if (sleep_trigger & I2C_TRIGGER)
          sleep_flag = 1;
      }      
    }
    else if (rx_data[0] == 0x41) {
      if (rx_data[1])
        cycle_status = 1;
      else
        cycle_status = 0;
      flash_data_write_back();
    }
    else if (rx_data[0] == 0xF0) {
      flash_data_write_back();
    }
    else if (rx_data[0] == 0xF1) {
      iap_switch = rx_data[1];
      flash_data_write_back();
    }
    else if (rx_data[0] == 0xFF) 
    {
      if (len == 2) {
        if (rx_data[1] < 128) {
          i2c_address[0] = rx_data[1];
          flash_data_write_back();
          user_i2c_init();
        }
      }
    }
    else if (rx_data[0] == 0xFD)
    {
      if (rx_data[1] == 1) {
        flag_jump_bootloader = 1;
        if (flag_jump_bootloader) {
          LL_I2C_DeInit(I2C1);
          LL_I2C_DisableAutoEndMode(I2C1);
          LL_I2C_Disable(I2C1);
          LL_I2C_DisableIT_ADDR(I2C1);
          LL_I2C_DeInit(I2C2);
          LL_I2C_DisableAutoEndMode(I2C2);
          LL_I2C_Disable(I2C2);
          LL_I2C_DisableIT_ADDR(I2C2);
          HAL_RTC_DeInit(&hrtc);
          i2c_port_set_to_input();
          while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) || HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7))
          {
            jump_bootloader_timeout++;
            if (jump_bootloader_timeout >= 60000) {
              flag_jump_bootloader = 0;
              break;
            }
          }
          if (jump_bootloader_timeout < 60000) {
            NVIC_SystemReset();
          } else {
            MX_GPIO_Init();
            MX_I2C2_Init();
            MX_RTC_Init();            
            user_i2c_init();
            i2c1_it_enable();        
            jump_bootloader_timeout = 0;
          }
        }        
      }   
    }         
  } 
  else if (len == 1) {
    if (rx_data[0] == 0x00) {
      i2c1_set_send_data((uint8_t *)&power_hold_status, 1);
    }
    else if (rx_data[0] == 0x01) {
      i2c1_set_send_data((uint8_t *)&led_status, 1);
    }
    else if ((rx_data[0] >= 0x10) && (rx_data[0] <= 0x11)) {
      buf[0] = my_button_a.button_status;
      buf[1] = my_button_b.button_status;
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x10], 0x11-rx_data[0]+1); 
    }
    else if ((rx_data[0] >= 0x20) && (rx_data[0] <= 0x21)) {
      if (rx_data[0] == 0x20) {
        buf[0] = wake_trigger;
        buf[1] = sleep_trigger;
        i2c1_set_send_data(buf, 2);
      }      
      else if (rx_data[0] == 0x21) {
        i2c1_set_send_data((uint8_t *)&sleep_trigger, 1);
      }      
    }
    else if ((rx_data[0] >= 0x30) && (rx_data[0] <= 0x32)) {
      buf[0] = my_rtc_data.rtc_hour_wake;
      buf[1] = my_rtc_data.rtc_min_wake;
      buf[2] = my_rtc_data.rtc_sec_wake;
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x30], 0x32-rx_data[0]+1);      
    }
    else if ((rx_data[0] >= 0x80) && (rx_data[0] <= 0x82)) {
      buf[0] = my_rtc_data.rtc_hour_sleep;
      buf[1] = my_rtc_data.rtc_min_sleep;
      buf[2] = my_rtc_data.rtc_sec_sleep;
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x80], 0x82-rx_data[0]+1);      
    }
    else if (rx_data[0] == 0x41) {
       i2c1_set_send_data((uint8_t *)&cycle_status,1);      
    }
    else if ((rx_data[0] >= 0x60) && (rx_data[0] <= 0x77)) {
      memcpy(&buf, (uint8_t *)&ina3221_send_data.vin, 4);
      memcpy(&buf[4], (uint8_t *)&ina3221_send_data.iin, 4);
      memcpy(&buf[4*2], (uint8_t *)&ina3221_send_data.vgrove, 4);
      memcpy(&buf[4*3], (uint8_t *)&ina3221_send_data.igrove, 4);
      memcpy(&buf[4*4], (uint8_t *)&ina3221_send_data.vbat, 4);
      memcpy(&buf[4*5], (uint8_t *)&ina3221_send_data.ibat, 4);
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x60], 0x77-rx_data[0]+1);      
    }
    else if (rx_data[0] == 0x90) {
      buf[0] = !HAL_GPIO_ReadPin(BAT_CHRG_GPIO_Port, BAT_CHRG_Pin);
      i2c1_set_send_data((uint8_t *)&buf[0], 1);
    }    
    else if (rx_data[0] == 0xFF) 
    {
      i2c1_set_send_data(i2c_address, 1);
    }    
    else if (rx_data[0] == 0xFE) 
    {
      i2c1_set_send_data((uint8_t *)&fm_version, 1);
    }    
    else if (rx_data[0] == 0xF1) {
      i2c1_set_send_data((uint8_t *)&iap_switch, 1);
    }     
  }  
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  IAP_Set();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  // MX_I2C1_Init();
  MX_I2C2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  clear_pwr_flag();
  HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port , PWR_HOLD_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(BL_EN_GPIO_Port , BL_EN_Pin, GPIO_PIN_SET);
  power_hold_status = 1;  
  INA3221SetConfig();
  init_flash_data();
  if ((!HAL_GPIO_ReadPin(BTN_B_GPIO_Port, BTN_B_Pin)) && (!HAL_GPIO_ReadPin(BTN_A_GPIO_Port, BTN_A_Pin))) {
    cycle_status = CYCLE_STOP;
    flash_data_write_back();
    HAL_Delay(100);
  }  
  u8g2Init(&u8g2); 
  u8g2_disp_init();
  if (led_status)
    HAL_GPIO_WritePin(BL_EN_GPIO_Port , BL_EN_Pin, GPIO_PIN_SET);
  else 
    HAL_GPIO_WritePin(BL_EN_GPIO_Port , BL_EN_Pin, GPIO_PIN_RESET);     
  user_i2c_init(); 
  i2c1_it_enable();  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_GetTick() > ina3221_delay_time) {
      ina3221_data.vin = getBusVoltage_V(2);
      ina3221_send_data.vin = (int32_t)(ina3221_data.vin * 100.0f);
      ina3221_data.vbat = getBusVoltage_V(3);
      ina3221_send_data.vbat = (int32_t)(ina3221_data.vbat * 100.0f);
      ina3221_data.vgrove = getBusVoltage_V(1);
      ina3221_send_data.vgrove = (int32_t)(ina3221_data.vgrove * 100.0f);
      ina3221_data.iin = getCurrent_mA(2);
      ina3221_send_data.iin = (int32_t)(ina3221_data.iin * 100.0f);
      ina3221_data.ibat = getCurrent_mA(3);
      ina3221_send_data.ibat = (int32_t)(ina3221_data.ibat * 100.0f);
      ina3221_data.igrove = getCurrent_mA(1);
      ina3221_send_data.igrove = (int32_t)(ina3221_data.igrove * 100.0f);
      ina3221_delay_time = HAL_GetTick() + INA3221_SAMPLE_TIME;
    }
    button_update();
    if (led_status) {
      switch (dis_status)
      {
      case DIS_MAIN:
        u8g2_disp_main();
        break;
      case DIS_MENU:
        u8g2_disp_menu_update();   
        break;
      
      default:
        break;
      }
    }

    if (excute_sleep_flag) {
      if (HAL_GetTick() - excute_sleep_delay >= display_after_sleep_time) {
        LL_I2C_DeInit(I2C1);
        LL_I2C_DisableAutoEndMode(I2C1);
        LL_I2C_Disable(I2C1);
        LL_I2C_DisableIT_ADDR(I2C1);   
        LL_I2C_DeInit(I2C2);
        LL_I2C_DisableAutoEndMode(I2C2);
        LL_I2C_Disable(I2C2);
        LL_I2C_DisableIT_ADDR(I2C2);        

        if (wake_trigger & TIMER_TRIGGER) {
          __HAL_RCC_PWR_CLK_ENABLE();
          HAL_PWREx_EnableInternalWakeUpLine();          
          __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUFI);
        }
        if (wake_trigger & BTN_TRIGGER) {
          HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
          HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW);
          __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
          __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
        }
        else {
          HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
          HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
        }        
        
        HAL_GPIO_WritePin(SYS_3V3_EN_GPIO_Port, SYS_3V3_EN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BL_EN_GPIO_Port, BL_EN_Pin, GPIO_PIN_RESET);
        led_status = 0;     
        HAL_PWR_EnterSTANDBYMode();
        excute_sleep_flag = 0;
      }
    }

    if (sleep_flag || (cycle_status && (my_rtc_data.rtc_hour_wake || my_rtc_data.rtc_min_wake || my_rtc_data.rtc_sec_wake))) {
      if ((!my_rtc_data.rtc_hour_wake && !my_rtc_data.rtc_min_wake && !my_rtc_data.rtc_sec_wake && (sleep_trigger & TIMER_TRIGGER))) {
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
              excute_sleep_delay = HAL_GetTick();
              excute_sleep_flag = 1; 
            }  				
          }
        }
        else {
          if (!rtc_sleep_time_enable) {
            if (my_rtc_data.rtc_hour_sleep || my_rtc_data.rtc_min_sleep || my_rtc_data.rtc_sec_sleep) {             
              set_RTC_Alarm(my_rtc_data.rtc_hour_sleep, my_rtc_data.rtc_min_sleep, my_rtc_data.rtc_sec_sleep);
              HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
              power_hold_status = 0;
              led_on_sleep_timeout_flag = 1;  
              rtc_sleep_time_enable = 1; 				
            }
          }
        }
      }
      else {
        if (((sleep_trigger & TIMER_TRIGGER) && (my_rtc_data.rtc_hour_wake || my_rtc_data.rtc_min_wake || my_rtc_data.rtc_sec_wake))) {
          if (!excute_sleep_flag) {
            if (!rtc_wake_time_enable && !led_on_sleep_timeout_flag) {
              HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_SET);
              power_hold_status = 1;               
              set_RTC_Alarm(my_rtc_data.rtc_hour_wake, my_rtc_data.rtc_min_wake, my_rtc_data.rtc_sec_wake);
              rtc_wake_time_enable = 1;						
            }
          }
        }
      }
    }
    else {
      HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
      rtc_wake_time_enable = 0;
    }     
    
    i2c_timeout_counter = 0;
    if (i2c_stop_timeout_flag) {
      if (i2c_stop_timeout_delay < HAL_GetTick()) {
        i2c_stop_timeout_counter++;
        i2c_stop_timeout_delay = HAL_GetTick() + 10;
      }
    }
    if (i2c_stop_timeout_counter > 50) {
      LL_I2C_DeInit(I2C1);
      LL_I2C_DisableAutoEndMode(I2C1);
      LL_I2C_Disable(I2C1);
      LL_I2C_DisableIT_ADDR(I2C1);     
      user_i2c_init();    
      i2c1_it_enable();
      HAL_Delay(500);
    }     
      
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  LL_PWR_EnableBkUpAccess();
  /* LSE configuration and activation */
  LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
  LL_RCC_LSE_Enable();
  while(LL_RCC_LSE_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the HSI */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(16000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
