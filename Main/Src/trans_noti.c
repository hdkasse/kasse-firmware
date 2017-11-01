
/**
  ******************************************************************************
  * @file    trans_noti.c
  * @author  Hyundai-pay Blockchain IOT Lab
  *          by nakjink@hyundai-pay.com
  ******************************************************************************
  * Permission is hereby granted, free of charge, to any person obtaining
  * a copy of this software and associated documentation files (the "Software"),
  * to deal in the Software without restriction, including without limitation
  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the
  * Software is furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included
  * in all copies or substantial portions of the Software.

  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "trans_noti.h"
#include "res_img.h"
#include "screen_disp.h"
#include "pincode.h"


static tran_noti_state_type noti_state = TRAN_NOTI_NONE_S;

void tran_noti_set_state(tran_noti_state_type state)
{
  noti_state = state;
}

void tran_noti_pre_sign_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(20,8,"Enter PIN",&Font_7x10);
    screen_disp_str(20,20,"for transaction",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_pin_for_transaction_90x32;
    else if(lang == JAP_FONT_E)str = (uint8_t*)j_pin_for_transaction_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void tran_noti_aft_sign_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(10,8,"The transaction",&Font_7x10);
    screen_disp_str(10,20,"is complete",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_transaction_complete_128x16;
    else if(lang == JAP_FONT_E)str = (uint8_t*)j_transaction_complete_128x16;
    
    screen_disp_bitmap(0,15,128,16,str);
  }
}

void tran_noti_state(uint32_t *evt_mask, uint8_t *lang, session_state_type *next_state)
{
  switch(noti_state)
  {
    case TRAN_NOTI_NONE_S:
      break;
    
    case TRAN_PRE_SIGN_INIT_S:
      tran_noti_pre_sign_init(*lang);
      noti_state = TRAN_PRE_SIGN_S;
      break;
    
    case TRAN_PRE_SIGN_S:
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
        noti_state = TRAN_CHECK_PIN_INIT_S;
      }
      else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        noti_state = TRAN_NOTI_NONE_S;
      }
      break;

    case TRAN_CHECK_PIN_INIT_S:
      simple_check_pin_init(lang);
      noti_state = TRAN_CHECK_PIN_S;
      break;
      
    case TRAN_CHECK_PIN_S:
      simple_check_pin(evt_mask, next_state, 0);
      if((*evt_mask & PIN_CHECK_OK_EVENT) == PIN_CHECK_OK_EVENT)
      {
        noti_state = TRAN_NOTI_NONE_S;
      }
      else if((*evt_mask & PIN_CHECK_NOTOK_EVENT) == PIN_CHECK_NOTOK_EVENT)
      {
        noti_state = TRAN_NOTI_NONE_S;
      }
      break;
      
    case TRAN_AFT_SIGN_INIT_S:
      tran_noti_aft_sign_init(*lang);
      noti_state = TRAN_AFT_SIGN_S;
      break;
    
    case TRAN_AFT_SIGN_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        noti_state = TRAN_NOTI_NONE_S;
      }
      break;
      
    default:
      break;
  }
}


void sec_update_notify_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
 

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(4,8,"Check & Resetting",&Font_7x10);
    screen_disp_str(4,20,"device changes",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_device_update_noti_128x28;
    else if(lang == JAP_FONT_E)str = (uint8_t*)h_device_update_noti_128x28;  // TBD....
    
    screen_disp_bitmap(0, 2, 128, 28, str);
  }
}


