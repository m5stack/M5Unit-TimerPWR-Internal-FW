/*

  u8g2_disp_fun.c

  Monochrome minimal user interface: Glue code between mui and u8g2.

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2021, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/
#include "u8g2_disp_fun.h"
#include "main.h"
#include "button.h"
#include "ina3221.h"
#include <string.h>
#include <math.h>
#include "rtc.h"

uint8_t dis_status = DIS_MAIN;

uint8_t main_status = 0;

uint8_t menu_wake_focus = 0;

uint8_t is_setting = 0;
uint8_t is_menu = 0;
uint8_t setting_index = 14;
uint8_t work_order = 0;
uint8_t notice_flag = 0;
uint8_t notice_counter = 0;
uint8_t oled_on_mode = 0;

typedef struct u8g2_disp_num {
    uint8_t wake_hour_0;
    uint8_t wake_hour_1;
    uint8_t wake_hour_2;
    uint8_t wake_min_0;
    uint8_t wake_min_1;
    uint8_t wake_sec_0;
    uint8_t wake_sec_1;
    uint8_t sleep_hour_0;
    uint8_t sleep_hour_1;
    uint8_t sleep_hour_2;
    uint8_t sleep_min_0;
    uint8_t sleep_min_1;
    uint8_t sleep_sec_0;
    uint8_t sleep_sec_1;
} u8g2_disp_num_t;

u8g2_disp_num_t my_disp_num;


const unsigned char gImage_mode_back[] = {
0x7E,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7E,
};

const unsigned char gImage_mode_start[] = {
0x04,0x00,0x0C,0x00,0x1C,0x00,0x3C,0x00,0x7C,0x00,0x3C,
0x00,0x1C,0x00,0x0C,0x00,0x04,0x00,
};

const unsigned char gImage_mode_stop[] = {
0x00,0x00,0x44,0x00,0x44,0x00,0x44,0x00,0x44,0x00,0x44,
0x00,0x44,0x00,0x44,0x00,0x00,0x00,
};

const unsigned char gImage_status[] = {
0x02,0x07,0x07,0x02,
};

unsigned char funIndex = 0;
void (*current)(void);

typedef struct
{
	uint8_t current;
	uint8_t up;//选择索引号
	uint8_t down;//选择索引号
	uint8_t exit;//退出索引号
	uint8_t enter;//确认索引号
	void (*current_operation)();
} key_table;

key_table table[30]=
{
	//第0层
	{0,3,1,3,0,(*u8g2_disp_menu_0_0)},
	{1,0,2,1,5,(*u8g2_disp_menu_0_1)},
	{2,1,3,0,6,(*u8g2_disp_menu_0_2)},
	{3,2,0,3,7,(*u8g2_disp_menu_0_3)},
	
  //第1层
	{4,4,4,0,4,(*u8g2_disp_menu_1_1)},					
	{5,5,5,1,5,(*u8g2_disp_menu_1_2)},
	{6,6,6,2,6,(*u8g2_disp_menu_1_1)},						                	
	{7,7,7,3,7,(*u8g2_disp_menu_1_4)},					                									
};

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    long res = 0;
    res = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if (res > out_max)
        return out_max;
    else if (res < out_min)
        return out_min;
    else
        return res;
}

void num_to_disp_num(void)
{
    my_disp_num.wake_hour_0 = my_rtc_data.rtc_hour_wake_temp / 100 % 10;
    my_disp_num.wake_hour_1 = my_rtc_data.rtc_hour_wake_temp / 10 % 10;
    my_disp_num.wake_hour_2 = my_rtc_data.rtc_hour_wake_temp % 10;
    my_disp_num.wake_min_0 = my_rtc_data.rtc_min_wake_temp / 10 % 10;
    my_disp_num.wake_min_1 = my_rtc_data.rtc_min_wake_temp % 10;
    my_disp_num.wake_sec_0 = my_rtc_data.rtc_sec_wake_temp / 10 % 10;
    my_disp_num.wake_sec_1 = my_rtc_data.rtc_sec_wake_temp % 10;

    my_disp_num.sleep_hour_0 = my_rtc_data.rtc_hour_sleep_temp / 100 % 10;
    my_disp_num.sleep_hour_1 = my_rtc_data.rtc_hour_sleep_temp / 10 % 10;
    my_disp_num.sleep_hour_2 = my_rtc_data.rtc_hour_sleep_temp % 10;
    my_disp_num.sleep_min_0 = my_rtc_data.rtc_min_sleep_temp / 10 % 10;
    my_disp_num.sleep_min_1 = my_rtc_data.rtc_min_sleep_temp % 10;
    my_disp_num.sleep_sec_0 = my_rtc_data.rtc_sec_sleep_temp / 10 % 10;
    my_disp_num.sleep_sec_1 = my_rtc_data.rtc_sec_sleep_temp % 10;
}

void disp_num_to_num(void)
{
    my_rtc_data.rtc_hour_wake_temp = my_disp_num.wake_hour_0 * 100 + my_disp_num.wake_hour_1 * 10 + my_disp_num.wake_hour_2;
    my_rtc_data.rtc_min_wake_temp = my_disp_num.wake_min_0 * 10 + my_disp_num.wake_min_1;
    my_rtc_data.rtc_sec_wake_temp = my_disp_num.wake_sec_0 * 10 + my_disp_num.wake_sec_1;

    my_rtc_data.rtc_hour_sleep_temp = my_disp_num.sleep_hour_0 * 100 + my_disp_num.sleep_hour_1 * 10 + my_disp_num.sleep_hour_2;
    my_rtc_data.rtc_min_sleep_temp = my_disp_num.sleep_min_0 * 10 + my_disp_num.sleep_min_1;
    my_rtc_data.rtc_sec_sleep_temp = my_disp_num.sleep_sec_0 * 10 + my_disp_num.sleep_sec_1;
}

void u8g2_disp_menu_update(void)
{
    current = table[funIndex].current_operation;//根据需要获取对应需要执行的函数
    (*current)();    
}

void u8g2_disp_init(void)
{
    my_rtc_data.rtc_hour_wake_temp = my_rtc_data.rtc_hour_wake;
    my_rtc_data.rtc_min_wake_temp = my_rtc_data.rtc_min_wake;
    my_rtc_data.rtc_sec_wake_temp = my_rtc_data.rtc_sec_wake;     
    my_rtc_data.rtc_hour_sleep_temp = my_rtc_data.rtc_hour_sleep;
    my_rtc_data.rtc_min_sleep_temp = my_rtc_data.rtc_min_sleep;
    my_rtc_data.rtc_sec_sleep_temp = my_rtc_data.rtc_sec_sleep; 

    num_to_disp_num();
}

void u8g2_disp_main(void)
{
    char dis_buffer[100] = {0};
    int buf_num = 0;
    static uint32_t display_delay = 0;
    static uint8_t display_flash_flag = 0;
    static uint32_t status_flash_counter = 0;
    static uint8_t status_flash_flag = 0;
    static uint32_t button_delay = 0;
    static uint32_t ab_button_delay = 0;
    static uint8_t is_press_init = 0;        
    static uint8_t is_ab_press_init = 0;    
    static uint8_t excute_flag = 0;    
    RTC_TimeTypeDef currentTime;
    RTC_DateTypeDef currentDate;
    RTC_AlarmTypeDef alarm;
    int remainingHours = 0;
    int remainingMinutes = 0;
    int remainingSeconds = 0;
    uint32_t currentTotalSeconds = 0, alarmTotalSeconds = 0, remainingTotalSeconds = 0;    

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&u8g2, 0); /*字体方向选择*/
   
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawHLine(&u8g2, 0, 7+16+3-2-1, 64);
    u8g2_DrawHLine(&u8g2, 0, 7+16+3+16-2, 64);
    u8g2_DrawVLine(&u8g2, 21, 0, 7+16+3-2);
    u8g2_DrawVLine(&u8g2, 43, 0, 7+16+3-2);

    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
    u8g2_DrawStr(&u8g2, 5, 6, "OUT"); 
    u8g2_DrawStr(&u8g2, 21+6, 6, "BAT"); 
    u8g2_DrawStr(&u8g2, 42+7, 6, "USB");

    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%.1fV", ina3221_data.vgrove);      
    u8g2_DrawStr(&u8g2, 0, 7+7+2-2, dis_buffer);  
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%.1fA", fabs(ina3221_data.igrove/1000.0f));    
    u8g2_DrawStr(&u8g2, 0, 7+7+2+7-2, dis_buffer);  
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%.1fV", ina3221_data.vbat);     
    u8g2_DrawStr(&u8g2, 21+2, 7+7+2-2, dis_buffer);  
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%.1fA", fabs(ina3221_data.ibat/1000.0f));     
    u8g2_DrawStr(&u8g2, 21+2, 7+7+2+7-2, dis_buffer);  
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%.1fV", ina3221_data.vin);      
    u8g2_DrawStr(&u8g2, 43+2, 7+7+2-2, dis_buffer);  
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%.1fA", fabs(ina3221_data.iin/1000.0f));     
    u8g2_DrawStr(&u8g2, 43+2, 7+7+2+7-2, dis_buffer);  
      
    if ((!cycle_status) && (!sleep_flag)) {
        if (!work_order) {
            if (setting_index <= 6 || setting_index == 14) {
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 7+16+3+7-2, ">");
            }
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7-2, "ON");
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_wake_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_wake_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_wake_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer); 

            if (setting_index > 6 && setting_index < 14) {
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 7+16+3+7+7-2, ">");
            }
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 5, 7+16+3+7+7-2, "OFF");
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_sleep_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_sleep_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_sleep_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer); 
        }
        else {
            if (setting_index <= 6 || setting_index == 14) {
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 7+16+3+7-2, ">");
            }
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7+7-2, "ON");
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_wake_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_wake_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_wake_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer); 

            if (setting_index > 6 && setting_index < 14) {
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 7+16+3+7+7-2, ">");
            }
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 5, 7+16+3+7-2, "OFF");
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_sleep_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_sleep_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
            memset(dis_buffer, 0, buf_num);
            buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_sleep_temp);  
            u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
            u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer);            
        }

    }
    else {
        if (0) {
            remainingHours = 0;
            remainingMinutes = 0;
            remainingSeconds = 0;
        }
        else {
            HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN);
            HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
            HAL_RTC_GetAlarm(&hrtc, &alarm, RTC_ALARM_A, RTC_FORMAT_BIN);   

            // 将当前时间和闹钟时间分别转换为总秒数
            currentTotalSeconds = (currentDate.Date - 1) * 86400 + currentTime.Hours * 3600 + currentTime.Minutes * 60 + currentTime.Seconds;
            alarmTotalSeconds = (alarm.AlarmDateWeekDay - 1) * 86400 + alarm.AlarmTime.Hours * 3600 + alarm.AlarmTime.Minutes * 60 + alarm.AlarmTime.Seconds;                       

            if (alarmTotalSeconds >= currentTotalSeconds) {
                remainingTotalSeconds = alarmTotalSeconds - currentTotalSeconds;
            }     

            remainingHours = remainingTotalSeconds / 3600;
            remainingMinutes = (remainingTotalSeconds % 3600) / 60;
            remainingSeconds = (remainingTotalSeconds % 3600) % 60;
        }

        if (!work_order) {
            memset(dis_buffer, 0, buf_num);
            if (!excute_sleep_flag) {
                if (!led_on_sleep_timeout_flag) {
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 0, 7+16+3+7-2, ">");
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7-2, "ON");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", remainingHours);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingMinutes);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingSeconds);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer); 
                }
                else {
                    // u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    // u8g2_DrawStr(&u8g2, 0, 7+16+3+7-2, ">");
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7-2, "ON");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_wake_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_wake_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_wake_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer);                     
                }
                if (led_on_sleep_timeout_flag) {
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 0, 7+16+3+7+7-2, ">");                
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 5, 7+16+3+7+7-2, "OFF");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", remainingHours);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingMinutes);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingSeconds);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer);               
                }
                else {
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 5, 7+16+3+7+7-2, "OFF");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_sleep_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_sleep_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_sleep_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer);          
                }
            }
            else {
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7-2, "ON");
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_wake_temp);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_wake_temp);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_wake_temp);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer);           

                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 7+16+3+7+7-2, ">");               
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 5, 7+16+3+7+7-2, "OFF");
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", remainingHours);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingMinutes);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingSeconds);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer);               
            }
        }
        else {
            if (!excute_sleep_flag) {
                memset(dis_buffer, 0, buf_num);
                if (!led_on_sleep_timeout_flag) {
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 0, 7+16+3+7+7-2, ">");
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7+7-2, "ON");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", remainingHours);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingMinutes);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingSeconds);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer); 
                }
                else {
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7+7-2, "ON");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_wake_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_wake_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_wake_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer);    
                }
                if (led_on_sleep_timeout_flag) {
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 0, 7+16+3+7-2, ">");                   
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 5, 7+16+3+7-2, "OFF");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", remainingHours);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingMinutes);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingSeconds);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer);           
                }
                else {
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 5, 7+16+3+7-2, "OFF");
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_sleep_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_sleep_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
                    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
                    memset(dis_buffer, 0, buf_num);
                    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_sleep_temp);  
                    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                    u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer);  
                }   
            }
            else {
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+5, 7+16+3+7+7-2, "ON");
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7+7-2, "=");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", my_rtc_data.rtc_hour_wake_temp);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_min_wake_temp);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", my_rtc_data.rtc_sec_wake_temp);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7+7-2, dis_buffer);

                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 7+16+3+7-2, ">");                 
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 5, 7+16+3+7-2, "OFF");
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+5, 7+16+3+7-2, "=");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03d", remainingHours);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+5, 7+16+3+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+5, 7+16+3+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingMinutes);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+5, 7+16+3+7-2, dis_buffer); 
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+5, 7+16+3+7-2, ":");
                memset(dis_buffer, 0, buf_num);
                buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02d", remainingSeconds);  
                u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
                u8g2_DrawStr(&u8g2, 4+10+4+3*5+3+2*5+3+5 , 7+16+3+7-2, dis_buffer);                  
            }
        }
    }

    if (display_delay < HAL_GetTick()) {
        display_delay = HAL_GetTick() + 200;
        if (display_flash_flag) {
            display_flash_flag = 0;
        }
        else {
            display_flash_flag = 1;
        }
        status_flash_counter++;
        notice_counter++;
        if (status_flash_counter % 3 == 0) {
            if (status_flash_flag) {
                status_flash_flag = 0;
            }
            else {
                status_flash_flag = 1;
            }            
        }
    }

    if (is_setting) {
        switch (setting_index)
        {
        case 0:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_hour_0++;
                else
                    my_disp_num.sleep_hour_0++;
                my_button_b.was_click = 0;
            }          
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_hour_0++;
                    else
                        my_disp_num.sleep_hour_0++;
                }        
            }    
            else
                is_press_init = 0; 

            if (!work_order) {
                if (my_disp_num.wake_hour_0 > 2)
                    my_disp_num.wake_hour_0 = 0; 
            }
            else {
                if (my_disp_num.sleep_hour_0 > 2)
                    my_disp_num.sleep_hour_0 = 0; 
            }
                               
            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21-1+3, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 1:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_hour_1++;
                else
                    my_disp_num.sleep_hour_1++;
                my_button_b.was_click = 0;
            }     
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_hour_1++;
                    else
                        my_disp_num.sleep_hour_1++;
                }        
            }  
            else
                is_press_init = 0;  
            if (!work_order) {
                if (my_disp_num.wake_hour_1 > 5)
                    my_disp_num.wake_hour_1 = 0;
            }
            else {
                if (my_disp_num.sleep_hour_1 > 5)
                    my_disp_num.sleep_hour_1 = 0;                
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5-1+3, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 2:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_hour_2++;
                else
                    my_disp_num.sleep_hour_2++;
                my_button_b.was_click = 0;
            }     
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_hour_2++;
                    else
                        my_disp_num.sleep_hour_2++;
                }        
            }   
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.wake_hour_2 > 5)
                    my_disp_num.wake_hour_2 = 0; 
            }
            else {
                if (my_disp_num.sleep_hour_2 > 5)
                    my_disp_num.sleep_hour_2 = 0; 
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5-1+3, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 3:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_min_0++;
                else
                    my_disp_num.sleep_min_0++;                
                my_button_b.was_click = 0;
            } 
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_min_0++;
                    else
                        my_disp_num.sleep_min_0++;  
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.wake_min_0 > 5)
                    my_disp_num.wake_min_0 = 0;
            }
            else {
                if (my_disp_num.sleep_min_0 > 5)
                    my_disp_num.sleep_min_0 = 0;
            }
                                        
            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5-1+3-3, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 4:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_min_1++;
                else
                    my_disp_num.sleep_min_1++;  
                my_button_b.was_click = 0;
            } 
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_min_1++;
                    else
                        my_disp_num.sleep_min_1++;  
                }        
            }  
            else
                is_press_init = 0; 

            if (!work_order) {
                if (my_disp_num.wake_min_1 > 9)
                    my_disp_num.wake_min_1 = 0;  
            }
            else {
                if (my_disp_num.sleep_min_1 > 9)
                    my_disp_num.sleep_min_1 = 0;                 
            }    

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5+5-1+3-3, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 5:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_sec_0++;
                else
                    my_disp_num.sleep_sec_0++;                
                my_button_b.was_click = 0;
            }           
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_sec_0++;
                    else
                        my_disp_num.sleep_sec_0++;  
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.wake_sec_0 > 5)
                    my_disp_num.wake_sec_0 = 0;
            }
            else {
                if (my_disp_num.sleep_sec_0 > 5)
                    my_disp_num.sleep_sec_0 = 0;                
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5+5+5+5-1+3-4, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 6:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.wake_sec_1++;
                else
                    my_disp_num.sleep_sec_1++; 
                my_button_b.was_click = 0;
            }           
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.wake_sec_1++;
                    else
                        my_disp_num.sleep_sec_1++;
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.wake_sec_1 > 9)
                    my_disp_num.wake_sec_1 = 0;
            }
            else {
                if (my_disp_num.sleep_sec_1 > 9)
                    my_disp_num.sleep_sec_1 = 0;
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5+5+5+5+5-1+3-4, 7+16+3-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 7:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_hour_0++;
                else
                    my_disp_num.wake_hour_0++;                  
                my_button_b.was_click = 0;
            }          
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_hour_0++;
                    else
                        my_disp_num.wake_hour_0++;  
                }        
            }    
            else
                is_press_init = 0; 

            if (!work_order) {
                if (my_disp_num.sleep_hour_0 > 2)
                    my_disp_num.sleep_hour_0 = 0;
            }
            else {
                if (my_disp_num.wake_hour_0 > 2)
                    my_disp_num.wake_hour_0 = 0;                
            }
                               
            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21-1+3, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;   
        case 8:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_hour_1++;
                else
                    my_disp_num.wake_hour_1++; 
                my_button_b.was_click = 0;
            }     
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_hour_1++;
                    else
                        my_disp_num.wake_hour_1++; 
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.sleep_hour_1 > 5)
                    my_disp_num.sleep_hour_1 = 0;
            }
            else {
                if (my_disp_num.wake_hour_1 > 5)
                    my_disp_num.wake_hour_1 = 0;                
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5-1+3, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 9:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_hour_2++;
                else
                    my_disp_num.wake_hour_2++; 
                my_button_b.was_click = 0;
            }     
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_hour_2++;
                    else
                        my_disp_num.wake_hour_2++; 
                }        
            }   
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.sleep_hour_2 > 5)
                    my_disp_num.sleep_hour_2 = 0;
            }
            else {
                if (my_disp_num.wake_hour_2 > 5)
                    my_disp_num.wake_hour_2 = 0;                
            }    

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5-1+3, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 10:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_min_0++;
                else
                    my_disp_num.wake_min_0++;                
                my_button_b.was_click = 0;
            } 
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_min_0++;
                    else
                        my_disp_num.wake_min_0++;
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.sleep_min_0 > 5)
                    my_disp_num.sleep_min_0 = 0; 
            }
            else {
                if (my_disp_num.wake_min_0 > 5)
                    my_disp_num.wake_min_0 = 0;                 
            }
                                        
            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5-1+3-3, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 11:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_min_1++;
                else
                    my_disp_num.wake_min_1++; 
                my_button_b.was_click = 0;
            } 
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_min_1++;
                    else
                        my_disp_num.wake_min_1++; 
                }        
            }  
            else
                is_press_init = 0; 

            if (!work_order) {
                if (my_disp_num.sleep_min_1 > 9)
                    my_disp_num.sleep_min_1 = 0; 
            }
            else {
                if (my_disp_num.wake_min_1 > 9)
                    my_disp_num.wake_min_1 = 0;                 
            }     

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5+5-1+3-3, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 12:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_sec_0++;
                else
                    my_disp_num.wake_sec_0++;                 
                my_button_b.was_click = 0;
            }           
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_sec_0++;
                    else
                        my_disp_num.wake_sec_0++;  
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.sleep_sec_0 > 5)
                    my_disp_num.sleep_sec_0 = 0; 
            }
            else {
                if (my_disp_num.wake_sec_0 > 5)
                    my_disp_num.wake_sec_0 = 0;                
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5+5+5+5-1+3-4, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;
        case 13:
            if (my_button_b.was_click) {
                if (!work_order)
                    my_disp_num.sleep_sec_1++;
                else
                    my_disp_num.wake_sec_1++; 
                my_button_b.was_click = 0;
            }           
            if (my_button_b.button_status == 0) {
                if (!is_press_init) {
                    button_delay = HAL_GetTick();
                    is_press_init = 1;
                }
                if (HAL_GetTick() - button_delay >= 300) {
                    if (!work_order)
                        my_disp_num.sleep_sec_1++;
                    else
                        my_disp_num.wake_sec_1++;
                }        
            }  
            else
                is_press_init = 0;  

            if (!work_order) {
                if (my_disp_num.sleep_sec_1 > 9)
                    my_disp_num.sleep_sec_1 = 0; 
            }
            else {
                if (my_disp_num.wake_sec_1 > 9)
                    my_disp_num.wake_sec_1 = 0;                
            }

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 21+5+5+5+5+5+5+5+5-1+3-4, 7+16+3+7-2, 5, 7);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;                     
        case 14:
            if (my_button_b.was_click) {
                if (work_order)
                    work_order = 0;
                else
                    work_order = 1;
            }            

            if (display_flash_flag) {
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 3, 7+16+3-2, 16, 14);
                u8g2_SetDrawColor(&u8g2, 1);
            }
            break;                     
        
        default:
            break;
        }
    }

    disp_num_to_num();

    if (my_rtc_data.rtc_min_sleep_temp > 59)
        my_rtc_data.rtc_min_sleep_temp = 0;
    if (my_rtc_data.rtc_sec_sleep_temp > 59)
        my_rtc_data.rtc_sec_sleep_temp = 0;
    if (my_rtc_data.rtc_min_wake_temp > 59)
        my_rtc_data.rtc_min_wake_temp = 0;
    if (my_rtc_data.rtc_sec_wake_temp > 59)
        my_rtc_data.rtc_sec_wake_temp = 0;
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    if ((!cycle_status) || (!my_rtc_data.rtc_hour_wake_temp && !my_rtc_data.rtc_min_wake_temp && !my_rtc_data.rtc_sec_wake_temp)) {
        u8g2_DrawStr(&u8g2, 14, 48, "<PAUSE>");
        u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
        if (!is_setting) {
            u8g2_DrawStr(&u8g2, 2, 48, "S");
        }
        else {
            u8g2_DrawStr(&u8g2, 1, 48, ">");
            if (setting_index < 14)
                u8g2_DrawStr(&u8g2, 64-5, 48, "+");            
            else if (setting_index == 14){
                if (!work_order) {
                    u8g2_DrawStr(&u8g2, 64-12, 48, "OFF");                                
                }
                else {
                    u8g2_DrawStr(&u8g2, 64-8, 48, "ON");                                
                }
            }
        }
    }
    else {
        u8g2_DrawStr(&u8g2, 19, 48, "<RUN>");
        if (my_button_a.is_longlongpressed) {
            notice_flag = 1;
            notice_counter = 0;
            my_button_a.is_longlongpressed = 0;
        }
        if (!notice_flag) {
            if (status_flash_flag) {
                u8g2_DrawXBMP(&u8g2, 64-10, 48-5, 3, 4, gImage_status);
            }
            else {
                u8g2_DrawXBMP(&u8g2, 64-10+4, 48-5, 3, 4, gImage_status);
            }  
        }
        else {
            if (display_flash_flag) {
                u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
                u8g2_DrawStr(&u8g2, 0, 48, ">>");
                u8g2_DrawStr(&u8g2, 64-8, 48, "<<");                
            }
            if (notice_counter > 4) {
                notice_flag = 0;
                notice_counter = 0;
            }
        }
    }

    u8g2_SendBuffer(&u8g2);

    if (my_button_a.was_click) {
        if (is_setting) {
            setting_index++;
            if (setting_index > 14)
                setting_index = 0;
        }
        my_button_a.was_click = 0;
    }     
    if (my_button_a.is_longlongpressed) {
        if (!cycle_status) {
            if (is_setting) {
                is_setting = 0;
                setting_index = 14;
            }
            else {
                is_setting = 1;
                setting_index = 14;
            }
        }
        my_button_a.is_longlongpressed = 0;
    }     
    if (my_button_b.was_click) {
        my_button_b.was_click = 0;
    }     
    if (my_button_b.is_longlongpressed) {
        my_button_b.is_longlongpressed = 0;
    }   
    if ((my_button_b.button_status == 0) && (my_button_a.button_status == 0)) {
        if (!is_ab_press_init) {
            ab_button_delay = HAL_GetTick();
            is_ab_press_init = 1;
        }
        if ((HAL_GetTick() - ab_button_delay >= 50) && (HAL_GetTick() - ab_button_delay < 150)) {
            if (sleep_trigger & BTN_TRIGGER) {
                excute_flag = 1;
            }
        }        
    }
    else
        is_ab_press_init = 0;  

    if (excute_flag) {
        excute_sleep_flag = 0; 
        HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
        rtc_sleep_time_enable = 0;
        rtc_wake_time_enable = 0;  
        led_on_sleep_timeout_flag = 0;      
        if (!my_rtc_data.rtc_hour_wake_temp && !my_rtc_data.rtc_min_wake_temp && !my_rtc_data.rtc_sec_wake_temp) {
            my_rtc_data.rtc_hour_wake = my_rtc_data.rtc_hour_wake_temp;
            my_rtc_data.rtc_min_wake = my_rtc_data.rtc_min_wake_temp;
            my_rtc_data.rtc_sec_wake = my_rtc_data.rtc_sec_wake_temp;     
            my_rtc_data.rtc_hour_sleep = my_rtc_data.rtc_hour_sleep_temp;
            my_rtc_data.rtc_min_sleep = my_rtc_data.rtc_min_sleep_temp;
            my_rtc_data.rtc_sec_sleep = my_rtc_data.rtc_sec_sleep_temp; 
            flash_data_write_back();   
            HAL_Delay(100);                          
            sleep_flag = 1;
        }
        else {
            if (!cycle_status) {
                cycle_status = 1;                                   
            }
            else {
                cycle_status = 0; 
            }
            is_setting = 0;
            setting_index = 14;            
            my_rtc_data.rtc_hour_wake = my_rtc_data.rtc_hour_wake_temp;
            my_rtc_data.rtc_min_wake = my_rtc_data.rtc_min_wake_temp;
            my_rtc_data.rtc_sec_wake = my_rtc_data.rtc_sec_wake_temp;     
            my_rtc_data.rtc_hour_sleep = my_rtc_data.rtc_hour_sleep_temp;
            my_rtc_data.rtc_min_sleep = my_rtc_data.rtc_min_sleep_temp;
            my_rtc_data.rtc_sec_sleep = my_rtc_data.rtc_sec_sleep_temp;    
            flash_data_write_back();   
            HAL_Delay(100); 
            if (work_order && cycle_status) {
                if (!oled_on_mode) {
                    if (((wake_trigger & TIMER_TRIGGER) && (my_rtc_data.rtc_hour_sleep || my_rtc_data.rtc_min_sleep || my_rtc_data.rtc_sec_sleep)) || (wake_trigger & BTN_TRIGGER)) {
                        HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);
                        power_hold_status = 0;
                        if (!excute_sleep_flag) {
                        if (wake_trigger & TIMER_TRIGGER) {
                            __HAL_RCC_PWR_CLK_ENABLE();
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
        }
        excute_flag = 0;
    }  
}

void u8g2_disp_menu_0_0(void)
{
    int buf_num = 0;
    char dis_buffer[100] = {0};
    static uint8_t setting_index = 0;

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&u8g2, 0); /*字体方向选择*/
   
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_DrawStr(&u8g2, 5, 8, "OLED Config"); 
    u8g2_DrawHLine(&u8g2, 0, 10, 64);
    u8g2_DrawHLine(&u8g2, 0, 7+16+3+16-2, 64);
    u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
    switch (setting_index)
    {
    case 0:
        if (!oled_on_mode) {
            u8g2_DrawStr(&u8g2, 2, 19, "> PowOff OFF"); 
            u8g2_DrawStr(&u8g2, 2, 28, "  Exit");
        }
        else {
            u8g2_DrawStr(&u8g2, 2, 19, "> PowOff ON"); 
            u8g2_DrawStr(&u8g2, 2, 28, "  Exit");            
        }

        if (my_button_b.was_click) {
            if (oled_on_mode)
                oled_on_mode = 0;
            else 
                oled_on_mode = 1;
            flash_data_write_back();
            HAL_Delay(100);
            my_button_b.was_click = 0;
        }         
        break;
    case 1:
        if (!oled_on_mode) {
            u8g2_DrawStr(&u8g2, 2, 19, "  PowOff OFF"); 
            u8g2_DrawStr(&u8g2, 2, 28, "> Exit");
        }
        else {
            u8g2_DrawStr(&u8g2, 2, 19, "  PowOff ON"); 
            u8g2_DrawStr(&u8g2, 2, 28, "> Exit");            
        }

        if (my_button_b.was_click) {
            dis_status = DIS_MAIN;
            setting_index = 0;
            my_button_b.was_click = 0;
        }         
        break;
    
    default:
        break;
    }
 
    u8g2_DrawStr(&u8g2, 0, 48, ">Next");
    u8g2_DrawStr(&u8g2, 64-15, 48, "Ok<");      
   
    u8g2_SendBuffer(&u8g2);  

    if (my_button_a.was_click) {
        setting_index++;
        if (setting_index > 1)
            setting_index = 0;
        my_button_a.was_click = 0;
    }
    if (my_button_a.is_longlongpressed) {
        my_button_a.is_longlongpressed = 0;
    }    
    if (my_button_b.was_click) {
        my_button_b.was_click = 0;
    }  
    if (my_button_b.is_longlongpressed) {
        my_button_b.is_longlongpressed = 0;
    }  
}

void u8g2_disp_menu_0_1(void)
{
    int buf_num = 0;
    char dis_buffer[100] = {0};

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&u8g2, 0); /*字体方向选择*/
   
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
    u8g2_DrawBox(&u8g2, 0, 0, 12, 9);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawStr(&u8g2, 1, 8, "ON"); 
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawHLine(&u8g2, 10, 8, 10);
    u8g2_DrawLine(&u8g2, 20, 8, 64-20, 40);
    u8g2_DrawHLine(&u8g2, 64-20, 40, 10);
    u8g2_DrawStr(&u8g2, 64-20+5, 39, "OFF"); 

    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03dh", my_rtc_data.rtc_hour_wake);
    u8g2_DrawStr(&u8g2, 0, 24, dis_buffer);   
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02dm%02ds", my_rtc_data.rtc_min_wake, my_rtc_data.rtc_sec_wake);
    u8g2_DrawStr(&u8g2, 0, 34, dis_buffer);   
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03dh", my_rtc_data.rtc_hour_sleep);    
    u8g2_DrawStr(&u8g2, 64-20, 26, dis_buffer);
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02dm%02ds", my_rtc_data.rtc_min_sleep, my_rtc_data.rtc_sec_sleep);    
    u8g2_DrawStr(&u8g2, 64-30, 16, dis_buffer);        

    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
    u8g2_DrawStr(&u8g2, 16, 48, "pausing");        
    u8g2_SendBuffer(&u8g2);  

    if (my_button_a.was_click) {
        funIndex = table[funIndex].down;
        my_button_a.was_click = 0;
    }
    if (my_button_a.is_longlongpressed) {
        my_button_a.is_longlongpressed = 0;
    }    
    if (my_button_b.was_click) {
        my_rtc_data.rtc_hour_wake_temp = my_rtc_data.rtc_hour_wake;
        my_rtc_data.rtc_min_wake_temp = my_rtc_data.rtc_min_wake;
        my_rtc_data.rtc_sec_wake_temp = my_rtc_data.rtc_sec_wake;
        funIndex = table[funIndex].enter;
        my_button_b.was_click = 0;
    }  
    if (my_button_b.is_longlongpressed) {
        my_button_b.is_longlongpressed = 0;
    }      
}

void u8g2_disp_menu_0_2(void)
{
    int buf_num = 0;
    char dis_buffer[100] = {0};

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&u8g2, 0); /*字体方向选择*/
   
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
    u8g2_DrawStr(&u8g2, 1, 8, "ON"); 
    u8g2_DrawHLine(&u8g2, 10, 8, 10);
    u8g2_DrawLine(&u8g2, 20, 8, 64-20, 40);
    u8g2_DrawHLine(&u8g2, 64-20, 40, 10);
    u8g2_DrawBox(&u8g2, 64-20+5-1, 39-8, 17, 9);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawStr(&u8g2, 64-20+5, 39, "OFF"); 
    u8g2_SetDrawColor(&u8g2, 1);

    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03dh", my_rtc_data.rtc_hour_wake);
    u8g2_DrawStr(&u8g2, 0, 24, dis_buffer);   
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02dm%02ds", my_rtc_data.rtc_min_wake, my_rtc_data.rtc_sec_wake);
    u8g2_DrawStr(&u8g2, 0, 34, dis_buffer);   
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%03dh", my_rtc_data.rtc_hour_sleep);    
    u8g2_DrawStr(&u8g2, 64-20, 26, dis_buffer);
    memset(dis_buffer, 0, buf_num);
    buf_num = snprintf(dis_buffer, sizeof(dis_buffer), "%02dm%02ds", my_rtc_data.rtc_min_sleep, my_rtc_data.rtc_sec_sleep);    
    u8g2_DrawStr(&u8g2, 64-30, 16, dis_buffer);   

    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
    u8g2_DrawStr(&u8g2, 16, 48, "pausing"); 

    u8g2_SendBuffer(&u8g2);  

    if (my_button_a.was_click) {
        funIndex = table[funIndex].exit;
        my_button_a.was_click = 0;
    }
    if (my_button_a.is_longlongpressed) {
        my_button_a.is_longlongpressed = 0;
    }    
    if (my_button_b.was_click) {
        my_rtc_data.rtc_hour_sleep_temp = my_rtc_data.rtc_hour_sleep;
        my_rtc_data.rtc_min_sleep_temp = my_rtc_data.rtc_min_sleep;
        my_rtc_data.rtc_sec_sleep_temp = my_rtc_data.rtc_sec_sleep;
        funIndex = table[funIndex].enter;
        my_button_b.was_click = 0;
    }    
    if (my_button_b.is_longlongpressed) {
        my_button_b.is_longlongpressed = 0;
    }      
}

void u8g2_disp_menu_0_3(void)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&u8g2, 0); /*字体方向选择*/
   
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawVLine(&u8g2, 10, 0, 48);

    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, 0, 10, 9, 50);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_m2icon_5_tf);
    u8g2_DrawGlyph(&u8g2, 1, 26+2+4, 0x0053); 
    u8g2_SetFont(&u8g2, u8g2_font_4x6_mf);
    u8g2_DrawStr(&u8g2, 2, 20+2-2+4, ".");    

    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawXBMP(&u8g2, 0, 0+1, 8, 8, gImage_mode_back);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
    u8g2_DrawStr(&u8g2, 2, 7+1, "M");
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 12, 11, "> BL");
    u8g2_SendBuffer(&u8g2);     

    if (my_button_a.was_click) {
        dis_status = DIS_MAIN;
        my_button_a.was_click = 0;
    }
    if (my_button_a.is_longlongpressed) {
        funIndex = table[funIndex].enter;
        my_button_a.is_longlongpressed = 0;
    }    
    if (my_button_b.was_click) {
        funIndex = table[funIndex].down;
        my_button_b.was_click = 0;
    }     
    if (my_button_b.is_longlongpressed) {
        my_button_b.is_longlongpressed = 0;
    }     
}

void u8g2_disp_menu_1_1(void)
{
    char dis_buffer[20] = {0};
    static uint8_t opt_index = 0;
    static uint32_t button_delay = 0;
    static uint8_t is_press_init = 0;
    static uint32_t display_delay = 0;
    static uint8_t display_flash_flag = 0;
  
    snprintf(dis_buffer, 20, "%03d:%02d:%02d", my_rtc_data.rtc_hour_wake_temp, my_rtc_data.rtc_min_wake_temp, my_rtc_data.rtc_sec_wake_temp);
		
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x13_mf);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawStr(&u8g2, 25, 13, "<Wake>"); 
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(&u8g2, 30, 21, ".Time"); 
    u8g2_DrawVLine(&u8g2, 17, 0, 48);
    u8g2_DrawBox(&u8g2, 19, 25, 46, 15);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawStr(&u8g2, 19, 36, dis_buffer); 
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawHLine(&u8g2, 0, 24, 17);
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
    if (my_button_a.was_click) {
        my_button_a.was_click = 0;
        opt_index++;
        is_press_init = 0;
        if (opt_index > 4)
            opt_index = 0;
    }     
    switch (opt_index)
    {
    case 0:
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save"); 
        u8g2_DrawVLine(&u8g2, 17, 0, 48);
        if (my_button_b.was_click) {
            my_button_b.was_click = 0;
        }  
        if (my_button_b.is_longlongpressed) {
            my_button_b.is_longlongpressed = 0;
        }                 
        break;
    case 1:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawVLine(&u8g2, 17, 0, 48);  
        if (my_button_b.was_click) {
            my_button_b.was_click = 0;
        }  
        if (my_button_b.is_longlongpressed) {
            my_button_b.is_longlongpressed = 0;
        }           
        break;
    case 2:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_DrawVLine(&u8g2, 17, 0, 48);  
        u8g2_DrawHLine(&u8g2, 0, 24, 17);
        if (display_delay < HAL_GetTick()) {
            display_delay = HAL_GetTick() + 200;
            if (display_flash_flag) {
                display_flash_flag = 0;
            }
            else {
                display_flash_flag = 1;
            }
        }
        if (display_flash_flag) {
            u8g2_DrawBox(&u8g2, 19, 25, 15, 15);
        }
        if (my_button_b.was_click) {
            my_button_b.was_click = 0;
            my_rtc_data.rtc_hour_wake_temp++;
        }
        if (my_button_b.button_status == 0) {
            if (!is_press_init) {
                button_delay = HAL_GetTick();
                is_press_init = 1;
            }
            if (HAL_GetTick() - button_delay >= 300) {
                my_rtc_data.rtc_hour_wake_temp++;
            }        
        }     
        else
            is_press_init = 0;                 
        break;
    case 3:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_DrawVLine(&u8g2, 17, 0, 48); 
        u8g2_DrawHLine(&u8g2, 0, 24, 17); 
        if (display_delay < HAL_GetTick()) {
            display_delay = HAL_GetTick() + 200;
            if (display_flash_flag) {
                display_flash_flag = 0;
            }
            else {
                display_flash_flag = 1;
            }
        }
        if (display_flash_flag) {
            u8g2_DrawBox(&u8g2, 19+20, 25, 10, 15);
        }
        if (my_button_b.was_click) {
            my_button_b.was_click = 0;
            my_rtc_data.rtc_min_wake_temp++;
        }  
        if (my_button_b.button_status == 0) {
            if (!is_press_init) {
                button_delay = HAL_GetTick();
                is_press_init = 1;
            }
            if (HAL_GetTick() - button_delay >= 300) {
                my_rtc_data.rtc_min_wake_temp++;
            }        
        }     
        else
            is_press_init = 0; 
        if (my_rtc_data.rtc_min_wake_temp > 59)
            my_rtc_data.rtc_min_wake_temp = 0;                
        break;
    case 4:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_DrawVLine(&u8g2, 17, 0, 48); 
        u8g2_DrawHLine(&u8g2, 0, 24, 17); 
        if (display_delay < HAL_GetTick()) {
            display_delay = HAL_GetTick() + 200;
            if (display_flash_flag) {
                display_flash_flag = 0;
            }
            else {
                display_flash_flag = 1;
            }
        }
        if (display_flash_flag) {
            u8g2_DrawBox(&u8g2, 19+20+15, 25, 10, 15);
        }
        if (my_button_b.was_click) {
            my_button_b.was_click = 0;
            my_rtc_data.rtc_sec_wake_temp++;
        }      
        if (my_button_b.button_status == 0) {
            if (!is_press_init) {
                button_delay = HAL_GetTick();
                is_press_init = 1;
            }
            if (HAL_GetTick() - button_delay >= 300) {
                my_rtc_data.rtc_sec_wake_temp++;
            }        
        }     
        else
            is_press_init = 0;
        if (my_rtc_data.rtc_sec_wake_temp > 59)
            my_rtc_data.rtc_sec_wake_temp = 0;                           
        break;
    
    default:
        break;
    }
    if (my_button_b.button_status == 0) {
        if (!is_press_init) {
            button_delay = HAL_GetTick();
            is_press_init = 1;
        }
        if (HAL_GetTick() - button_delay >= 300) {
            if (opt_index == 0) {
                uint8_t x_map_value = map(HAL_GetTick() - button_delay, 300, 1300, 0, 45);
                if (x_map_value < 45)
                    u8g2_DrawBox(&u8g2, 19, 43, x_map_value, 5);
                else {
                    opt_index = 0;
                    my_button_a.was_click = 0;
                    my_button_a.is_longlongpressed = 0;
                    my_button_b.was_click = 0;
                    my_button_b.is_longlongpressed = 0;
                    funIndex = table[funIndex].exit;
                }
            }
            else if (opt_index == 1) {
                uint8_t x_map_value = map(HAL_GetTick() - button_delay, 300, 3300, 0, 45);
                if (x_map_value < 45)
                    u8g2_DrawBox(&u8g2, 19, 43, x_map_value, 5);
                else {
                    opt_index = 0;
                    my_button_a.was_click = 0;
                    my_button_a.is_longlongpressed = 0;
                    my_button_b.was_click = 0;
                    my_button_b.is_longlongpressed = 0;
                    my_rtc_data.rtc_hour_wake = my_rtc_data.rtc_hour_wake_temp;
                    my_rtc_data.rtc_min_wake = my_rtc_data.rtc_min_wake_temp;
                    my_rtc_data.rtc_sec_wake = my_rtc_data.rtc_sec_wake_temp;                    
                    funIndex = table[funIndex].exit;
                }
            }
        }        
    }     
    else
        is_press_init = 0;
     
    u8g2_SendBuffer(&u8g2); 
}

void u8g2_disp_menu_1_2(void)
{
    int buf_num = 0;
    char dis_buffer[100] = {0};

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 1); /*字体模式选择*/
    u8g2_SetFontDirection(&u8g2, 0); /*字体方向选择*/
   
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
    u8g2_DrawStr(&u8g2, 2, 9, "QUIT"); 
    u8g2_DrawStr(&u8g2, 4, 27, "ON 000:00:00"); 
    u8g2_DrawStr(&u8g2, 4, 46, "OK"); 
    u8g2_DrawStr(&u8g2, 42, 46, "QUIT"); 

    u8g2_SendBuffer(&u8g2);  

    if (my_button_a.was_click) {
        dis_status = DIS_MAIN;
        my_button_a.was_click = 0;
    }
    if (my_button_a.is_longlongpressed) {
        my_rtc_data.rtc_hour_wake_temp = my_rtc_data.rtc_hour_wake;
        my_rtc_data.rtc_min_wake_temp = my_rtc_data.rtc_min_wake;
        my_rtc_data.rtc_sec_wake_temp = my_rtc_data.rtc_sec_wake;
        funIndex = table[funIndex].enter;
        my_button_a.is_longlongpressed = 0;
    }    
    if (my_button_b.was_click) {
        funIndex = table[funIndex].down;
        my_button_b.was_click = 0;
    }    
    if (my_button_b.is_longlongpressed) {
        my_button_b.is_longlongpressed = 0;
    }      
}

void u8g2_disp_menu_1_3(void)
{
    char dis_buffer[20] = {0};
    static uint8_t opt_index = 0;
    static uint32_t button_delay = 0;
    static uint8_t is_press_init = 0;
    static uint32_t display_delay = 0;
    static uint8_t display_flash_flag = 0;
  
    int buf_nums = snprintf(dis_buffer, 20, "Charge");
    if (buf_nums > 9) buf_nums = 9;
		
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x13_mf);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawStr(&u8g2, 22, 13, "<USBIN>"); 
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tf);
    u8g2_DrawVLine(&u8g2, 17, 0, 48);
    u8g2_DrawBox(&u8g2, 19, 25, 46, 15);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawStr(&u8g2, 19+(45-buf_nums*5)/2, 36, dis_buffer); 
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawHLine(&u8g2, 0, 24, 17);
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
    if (my_button_a.was_click) {
        my_button_a.was_click = 0;
        opt_index++;
        if (opt_index > 2)
            opt_index = 0;
    }     
    switch (opt_index)
    {
    case 0:
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save"); 
        u8g2_DrawVLine(&u8g2, 17, 0, 48);
        break;
    case 1:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawVLine(&u8g2, 17, 0, 48);  
        break;
    case 2:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_DrawVLine(&u8g2, 17, 0, 48);  
        u8g2_DrawHLine(&u8g2, 0, 24, 17);
        if (display_delay < HAL_GetTick()) {
            display_delay = HAL_GetTick() + 200;
            if (display_flash_flag) {
                display_flash_flag = 0;
            }
            else {
                display_flash_flag = 1;
            }
        }
        if (display_flash_flag) {
            u8g2_DrawBox(&u8g2, 19, 25, 46, 15);
        }
        break;
    
    default:
        break;
    }
     
    u8g2_SendBuffer(&u8g2); 
}

void u8g2_disp_menu_1_4(void)
{
    char dis_buffer[20] = {0};
    static uint8_t opt_index = 0;
    static uint32_t button_delay = 0;
    static uint8_t is_press_init = 0;
    static uint32_t display_delay = 0;
    static uint8_t display_flash_flag = 0;
  
    int buf_nums = snprintf(dis_buffer, 20, "Manual");
    if (buf_nums > 9) buf_nums = 9;
		
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x13_mf);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawStr(&u8g2, 27, 13, "<Run>"); 
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(&u8g2, 30, 21, ".Mode"); 
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tf);
    u8g2_DrawVLine(&u8g2, 17, 0, 48);
    u8g2_DrawBox(&u8g2, 19, 25, 46, 15);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawStr(&u8g2, 19+(45-buf_nums*5)/2, 36, dis_buffer); 
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawHLine(&u8g2, 0, 24, 17);
    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
    if (my_button_a.was_click) {
        my_button_a.was_click = 0;
        opt_index++;
        if (opt_index > 2)
            opt_index = 0;
    }     
    switch (opt_index)
    {
    case 0:
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save"); 
        u8g2_DrawVLine(&u8g2, 17, 0, 48);
        break;
    case 1:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawVLine(&u8g2, 17, 0, 48);  
        break;
    case 2:
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 0, 0, 17, 24);
        u8g2_DrawBox(&u8g2, 0, 24, 17, 24);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, 1, 15, "Quit");
        u8g2_DrawStr(&u8g2, 1, 24+15, "Save");  
        u8g2_DrawVLine(&u8g2, 17, 0, 48);  
        u8g2_DrawHLine(&u8g2, 0, 24, 17);
        if (display_delay < HAL_GetTick()) {
            display_delay = HAL_GetTick() + 200;
            if (display_flash_flag) {
                display_flash_flag = 0;
            }
            else {
                display_flash_flag = 1;
            }
        }
        if (display_flash_flag) {
            u8g2_DrawBox(&u8g2, 19, 25, 46, 15);
        }
        break;
    
    default:
        break;
    }
    u8g2_SendBuffer(&u8g2); 
}
