/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "button.h"

uint8_t button_enable_flag = 1;
uint8_t button_init_flag = 0;

button_t my_button_a;
button_t my_button_b;

uint8_t read_button_a_status(uint8_t *button_status)
{
  uint16_t counter = 0, gpio_p = 0;
  uint8_t button_s = 0; 
  uint8_t last_button_s = 0;
    
  last_button_s = HAL_GPIO_ReadPin(BTN_A_GPIO_Port, BTN_A_Pin);
  for (uint16_t i = 0; i < BUTTON_FILTER_TIMEROUT; i++) {  
    button_s = HAL_GPIO_ReadPin(BTN_A_GPIO_Port, BTN_A_Pin);
    if (button_s == last_button_s) {
      counter++;
    }
    else {
      last_button_s = button_s;
      counter = 0;
    }
    if (counter >= BUTTON_FILTER) {
      *button_status = button_s;
      return 1;
    }
  }
  return 0;
}

uint8_t read_button_b_status(uint8_t *button_status)
{
  uint16_t counter = 0, gpio_p = 0;
  uint8_t button_s = 0; 
  uint8_t last_button_s = 0;
    
  last_button_s = HAL_GPIO_ReadPin(BTN_B_GPIO_Port, BTN_B_Pin);
  for (uint16_t i = 0; i < BUTTON_FILTER_TIMEROUT; i++) {  
    button_s = HAL_GPIO_ReadPin(BTN_B_GPIO_Port, BTN_B_Pin);
    if (button_s == last_button_s) {
      counter++;
    }
    else {
      last_button_s = button_s;
      counter = 0;
    }
    if (counter >= BUTTON_FILTER) {
      *button_status = button_s;
      return 1;
    }
  }
  return 0;
}

void button_update(void)
{
  static uint8_t is_a_long_pressing = 0;
  static uint8_t is_b_long_pressing = 0;

  if (button_enable_flag) {
    if(!button_init_flag) {
      my_button_a.button_status = 1;
      my_button_a.button_delay = 0;
      my_button_a.is_pressed = 0;
      my_button_a.is_longlongpressed = 0;
      my_button_a.was_click = 0;
      my_button_a.was_longpress = 0;
      my_button_a.was_longlongpress = 0;

      my_button_b.button_status = 1;
      my_button_b.button_delay = 0;
      my_button_b.is_pressed = 0;
      my_button_b.is_longlongpressed = 0;
      my_button_b.was_click = 0;
      my_button_b.was_longpress = 0;
      my_button_b.was_longlongpress = 0;
      button_init_flag = 1;
    } 

    if (read_button_a_status(&my_button_a.button_status)) {
      if (!my_button_a.is_pressed && !my_button_a.button_status) {
        my_button_a.is_pressed = 1;
        my_button_a.button_delay = HAL_GetTick();
      }
      if (my_button_a.is_pressed && !my_button_a.button_status) {
        if (HAL_GetTick() - my_button_a.button_delay > 1000) {
          if (!is_a_long_pressing) {
            my_button_a.is_longlongpressed = 1;
            is_a_long_pressing = 1;
          }
        }
      }     
      if (is_a_long_pressing) {
        if (my_button_a.button_status) {
          is_a_long_pressing = 0;
          my_button_a.is_pressed = 0;
        }
      }
      else {
        if (my_button_a.is_pressed && my_button_a.button_status) {
          if (HAL_GetTick() - my_button_a.button_delay >= 50 && HAL_GetTick() - my_button_a.button_delay <= 500)
            my_button_a.was_click = 1;
          else if (HAL_GetTick() - my_button_a.button_delay >= 2000 && HAL_GetTick() - my_button_a.button_delay <= 5000)
            my_button_a.was_longpress = 1;
          else if (HAL_GetTick() - my_button_a.button_delay > 5000)
            my_button_a.was_longlongpress = 1;
          my_button_a.is_pressed = 0;
        }
      }
    }

    if (read_button_b_status(&my_button_b.button_status)) {
      if (!my_button_b.is_pressed && !my_button_b.button_status) {
        my_button_b.is_pressed = 1;
        my_button_b.button_delay = HAL_GetTick();
      }
      if (my_button_b.is_pressed && !my_button_b.button_status) {
        if (HAL_GetTick() - my_button_b.button_delay > 1000) {
          if (!is_b_long_pressing) {
            my_button_b.is_longlongpressed = 1;
            is_b_long_pressing = 1;
          }
        }
      }     
      if (is_b_long_pressing) {
        if (my_button_b.button_status) {
          is_b_long_pressing = 0;
          my_button_b.is_pressed = 0;
        }
      }
      else {
        if (my_button_b.is_pressed && my_button_b.button_status) {
          if (HAL_GetTick() - my_button_b.button_delay >= 50 && HAL_GetTick() - my_button_b.button_delay <= 500)
            my_button_b.was_click = 1;
          else if (HAL_GetTick() - my_button_b.button_delay >= 2000 && HAL_GetTick() - my_button_b.button_delay <= 5000)
            my_button_b.was_longpress = 1;
          else if (HAL_GetTick() - my_button_b.button_delay > 5000)
            my_button_b.was_longlongpress = 1;
          my_button_b.is_pressed = 0;
        }
      }
    }
  }
}