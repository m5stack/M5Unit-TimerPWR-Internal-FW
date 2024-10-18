/*

  u8g2_disp_fun.h

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
#ifndef __U8G2_DISP_FUN_H__
#define __U8G2_DISP_FUN_H__

#include "oled_u8g2.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

enum {DIS_MAIN = 0, DIS_MENU};

extern uint8_t dis_status;
extern uint8_t btnAflag;
extern uint8_t btnBflag;
extern uint8_t btnALongflag;
extern uint8_t btnBLongflag;
extern uint8_t work_order;
extern uint8_t oled_on_mode;

void u8g2_disp_init(void);

void u8g2_disp_main(void);
void u8g2_disp_menu_0_0(void);
void u8g2_disp_menu_0_1(void);
void u8g2_disp_menu_0_2(void);
void u8g2_disp_menu_0_3(void);
void u8g2_disp_menu_1_1(void);
void u8g2_disp_menu_1_2(void);
void u8g2_disp_menu_1_3(void);
void u8g2_disp_menu_1_4(void);
void u8g2_disp_menu_update(void);
void num_to_disp_num(void);

#endif