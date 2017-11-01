/**
  ******************************************************************************
  * @file    pincode.c
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

#include <stdint.h>
#include "user_type.h"
#include "pincode.h"
#include "screen_disp.h"
#include "res_img.h"
#include "flash_dload.h"
#include "apdu_i2c.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_PIN_RETRY 10

typedef struct
{
  uint8_t    code[10];
  uint8_t    curr_pos;
}input_pincode_type;

static uint8_t pinValidation = 0;

static input_pincode_type incode;

static uint8_t pin_gen_count = 0;
static input_pincode_type incode2;

static uint8_t pin_number = 0;
static process_type p_type = NONE_PROCESS;

static pin_state_type pin_state = NONE_STATE;

static pin_check_owner_type pinowner = OWNER_NONE;

extern device_feature_type dev_feature;


void pincode_set_type(process_type ptype)
{
  p_type = ptype;
}

process_type pincode_get_type(void)
{
  return p_type;
}

void pincode_set_state(pin_state_type state)
{
  pin_state = state;
}

uint8_t pincode_validation(void)
{
  return pinValidation;
}

uint8_t pincode_is_cached(void)
{
  uint8_t is_cached = FALSE;
  
  if(dev_feature.pin_cached > 0)
  {
    is_cached = TRUE;
  }

  return is_cached;
}

/*
   Normal PIN code verfication step.
*/
void pincode_verify_init(uint8_t *lang)
{
  screen_disp_clear();
  // TBD....
}

void pincode_verify_failure(uint8_t *lang, uint8_t count)
{
  uint8_t *str = NULL;
  screen_disp_clear();

  if(count == 0xff)
  {
    screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
    if(*lang == ENG_FONT_E)
    {
      screen_disp_str(15,8,"Invalid PIN",&Font_7x10);
      screen_disp_str(15,20,"Please retry",&Font_7x10);
    }
    else
    {
      if(*lang == KOR_FONT_E)str = (uint8_t*)h_invalid_pin_90x32;
      else if(*lang == JAP_FONT_E)str = (uint8_t*)j_invalid_pin_90x32;
      
      screen_disp_bitmap(19,0,90,32,str);
    }
  }
  else
  {
    screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
    if(*lang == ENG_FONT_E)
    {
      screen_disp_str(15,8,"Invalid PIN",&Font_7x10);
      screen_disp_str(15,20,"Remain #",&Font_7x10);
      screen_disp_num(78,20, count, &Font_7x10);
    }
    else
    {
      if(*lang == KOR_FONT_E)
      {
        screen_disp_bitmap(5, 0, 100, 16, (uint8_t*)h_invalid_pin_remain_100x16);
        screen_disp_bitmap(10, 16, 60, 16, (uint8_t*)h_remain_retry_60x16);
        screen_disp_num(80, 20, count, &Font_7x10);
      }
      else if(*lang == JAP_FONT_E)
      {
        screen_disp_bitmap(5, 0, 100, 16, (uint8_t*)j_invalid_pin_remain_100x16);
        screen_disp_bitmap(10, 16, 60, 16, (uint8_t*)j_remain_retry_60x16);
        screen_disp_num(80, 18, count, &Font_7x10);
      }
    }
  }
}

void pincode_verify_perm_fail(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(1,8,"Failed 10 times",&Font_7x10);
    screen_disp_str(1,20,"Wallet is reset",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_retry_fail_wallet_init_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_retry_fail_wallet_init_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }

}


void pincode_generate_complete(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(10,8,"New PIN code",&Font_7x10);
    screen_disp_str(10,20,"Complete",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_pin_gen_ok_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_pin_gen_ok_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void pincode_generate_1_more(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();

  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(p_type != PROC_PIN_1ST_GEN)
  {
    screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
  }

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(15,8,"Enter again",&Font_7x10);
    screen_disp_str(15,20,"Same PIN code",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_input_again_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_input_again_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void pincode_generate_failure(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();

  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(15,8,"Invalid PIN",&Font_7x10);
    screen_disp_str(15,20,"Please retry",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_invalid_pin_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_invalid_pin_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void pincode_1st_gen_failure(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();
  
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(15,8,"Invalid PIN",&Font_7x10);
    screen_disp_str(15,20,"Please retry",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_invalid_pin_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_invalid_pin_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void pin_generate_init(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(22,8,"New PIN code",&Font_7x10);
    screen_disp_str(22,20,"Please enter",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_new_pin_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_new_pin_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void pin_empty_init(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  //screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  if(*lang == ENG_FONT_E)
  {
    screen_disp_str(22,8,"Require PIN",&Font_7x10);
    screen_disp_str(22,20,"generation",&Font_7x10);
  }
  else
  {
    if(*lang == KOR_FONT_E)str = (uint8_t*)h_empty_pin_90x32;
    else if(*lang == JAP_FONT_E)str = (uint8_t*)j_empty_pin_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void pin_input_disp(uint8_t *lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();

  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_up_7x4);
  //screen_disp_bitmap(55, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);


  if(*lang == KOR_FONT_E)str = (uint8_t*)h_pincode_79x16;
  else if(*lang == ENG_FONT_E)str = (uint8_t*)e_pincode_79x16;
  else if(*lang == JAP_FONT_E)str = (uint8_t*)j_pincode_79x16;

  screen_disp_bitmap(24,1,79,16,str);

  screen_disp_code_init();
  screen_disp_underline(incode.curr_pos+1, 0);
  screen_disp_code_num(incode.curr_pos+1, 0);
  pin_number = 0;
}

void input_pin_init(uint8_t *lang)
{
  memset(&incode, 0x0, sizeof(input_pincode_type));

  screen_disp_clear();

  if(p_type == PROC_PIN_VERIFY || p_type == PROC_CHANGE_PIN)
  {
    pin_state = PIN_INPUT_DISP;
  }
  else if(p_type == PROC_PIN_1ST_GEN)
  {
    if(pin_gen_count < 2)
    {
      pin_state = PIN_INPUT_DISP;
    }
    else
    {
      pincode_generate_1_more(lang);
      pin_state = PIN_GEN_2ND_DISP;
    }
  }
  else if(p_type == PROC_PIN_GENERATE)
  {
    if(pin_gen_count < 2)
    {
      pin_generate_init(lang);	  
      pin_state = PIN_GEN_1ST_DISP;
    }
    else
    {
      pincode_generate_1_more(lang);
      pin_state = PIN_GEN_2ND_DISP;
    }
  }
}

void short_input_pin_fail(uint8_t *lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  
  if(*lang == KOR_FONT_E)str = (uint8_t*)h_short_pin_90x28;
  else if(*lang == ENG_FONT_E)str = (uint8_t*)e_short_pin_90x28;
  else if(*lang == JAP_FONT_E)str = (uint8_t*)h_short_pin_90x28; // TBD....
  
  screen_disp_bitmap(18,2,90,28,str);
}
void short_input_pin_noti(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    if(p_type == PROC_PIN_1ST_GEN)
    {
      pin_state = PIN_EMPTY_INIT;
    }
    else if(p_type == PROC_PIN_VERIFY)
    {
      pin_state = PIN_NONE_STATE;
      *evt_mask |= PIN_CHECK_NOTOK_EVENT;
    }
    else
    {
      pin_state = PIN_NONE_STATE;
      *evt_mask |= PIN_INPUT_CANCEL_EVENT;
    }
  }
}

void pin_gen_1st_disp(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;  
    pin_state = PIN_INPUT_DISP;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    if(p_type != PROC_PIN_1ST_GEN)
    {
      *evt_mask |= PIN_INPUT_CANCEL_EVENT;
      pin_state = PIN_NONE_STATE;
    }
  }
}
void pin_gen_2nd_disp(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;  
    pin_state = PIN_INPUT_DISP;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    if(p_type != PROC_PIN_1ST_GEN)
    {
      *evt_mask |= PIN_INPUT_CANCEL_EVENT;
      pin_state = PIN_NONE_STATE;
    }
  }
}

void input_pin_code(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(incode.curr_pos < CODE_NUMBER)
    pin_number++;
    if(pin_number > 9)pin_number = 0;
    
    screen_disp_underline(incode.curr_pos+1, 0);
    screen_disp_code_num(incode.curr_pos+1, pin_number);
    *evt_mask &= ~BUTTON_1_EVENT;
  
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT) // confirm 1 number
  {
    incode.code[incode.curr_pos] = pin_number;
    screen_disp_code_char(incode.curr_pos+1, '*');
	
    incode.curr_pos++;
    if(incode.curr_pos < CODE_NUMBER)
    {
      pin_number = 0;
      screen_disp_underline(incode.curr_pos+1, 0);
      screen_disp_code_num(incode.curr_pos+1, pin_number);
    }
    else
    {
      incode.curr_pos--;
    }
    
    *evt_mask &= ~BUTTON_2_EVENT;
  
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT) // input complete
  {
    int i;
    incode.code[incode.curr_pos] = pin_number;
    screen_disp_code_char(incode.curr_pos+1, '*');

    for(i=incode.curr_pos;i<CODE_NUMBER;i++)
    {
      screen_disp_underline(i+1, 0);
    }
	
    *evt_mask &= ~BUTTON_1_2_EVENT;

    if(p_type == PROC_PIN_VERIFY)
    {
      if((incode.curr_pos+1) < 4)
      {
        pincode_set_state(PIN_SHORT_INPUT_INIT);
      }
      else
      {
        pincode_set_state(PIN_VERIFY);
      }
    }
    else if(p_type == PROC_PIN_GENERATE || p_type == PROC_PIN_1ST_GEN)
    {
      pincode_set_state(PIN_GENERATE);
    }
    else if(p_type == PROC_CHANGE_PIN)
    {
      pincode_set_state(PIN_VERIFY);
    }  
  }
  else if((*evt_mask & BUTTON_2_LONG_EVENT) == BUTTON_2_LONG_EVENT)  // backspace
  {
    if(incode.curr_pos > 0)
    {
      pin_number = incode.code[incode.curr_pos-1];
      incode.code[incode.curr_pos] = 0;
      screen_disp_code_char(incode.curr_pos+1, ' ');
      screen_disp_underline(incode.curr_pos+1, 1);
      screen_disp_underline(incode.curr_pos+1, 1);
      screen_disp_code_num(incode.curr_pos, pin_number);
      incode.curr_pos--; 
    }
    *evt_mask &= ~BUTTON_2_LONG_EVENT;
  
  }
  
}


void pin_state_mach(uint32_t *evt_mask, uint8_t *lang)
{
  uint8_t apdu_result=0, retry_cnt=0;

  switch(pin_state)
  {
    case PIN_NONE_STATE:
      break;

    case PIN_VERIFY_INIT:
      pincode_verify_init(lang);
      pin_state = PIN_INPUT_INIT;
      break;

    case PIN_VERIFY:
      if(p_type == PROC_CHANGE_PIN)
      {
        apdu_result = (!memcmp(incode.code, dev_feature.pin_status.pin_code, sizeof(incode.curr_pos+1))
                       && ((incode.curr_pos+1) == dev_feature.pin_status.pin_len));
        if(apdu_result)
        {
          pin_state = PIN_VERIFY_OK;
        }
        else
        {
          pin_state = PIN_VERIFY_INVALID;
          pincode_verify_failure(lang, 0xff);
        }
      }
      else
      {
        apdu_result = APDU_set_pin_check(APDU_PIN_VERIFY, incode.code, incode.curr_pos+1, &retry_cnt);

        if(apdu_result)
        {
          dev_feature.pin_status.pin_len = incode.curr_pos+1;
          memcpy(dev_feature.pin_status.pin_code, incode.code, incode.curr_pos+1);
          
          pin_state = PIN_VERIFY_OK;
          pinValidation = 1;
        }
        else
        {
          if(retry_cnt > 0)
          {
            pin_state = PIN_VERIFY_INVALID;
            pincode_verify_failure(lang, retry_cnt);
            pinValidation = 0;
            dev_feature.pin_status.retry_cnt = retry_cnt;
          }
          else
          {
            flash_fd_pin_permanent_fail();
            dev_feature.pin_status.retry_cnt = 0;
            pinValidation = 0;
            pincode_verify_perm_fail(lang);
            pin_state = PIN_VERIFY_PERM_FAIL;
          }
        }
      }
      break;

    case PIN_VERIFY_OK:
      pin_state = PIN_NONE_STATE;
      *evt_mask |= PIN_CHECK_OK_EVENT;
      break;

    case PIN_VERIFY_INVALID:
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
      }
      else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        pin_state = PIN_NONE_STATE;
        *evt_mask &= ~BUTTON_2_EVENT;
        *evt_mask |= PIN_CHECK_NOTOK_EVENT;
      }
      break;

    case PIN_EMPTY_INIT:
      pin_empty_init(lang);
      pin_state = PIN_EMPTY_NOTI;
      break;

    case PIN_EMPTY_NOTI:
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
        pin_state = PIN_INPUT_INIT;
        p_type = PROC_PIN_1ST_GEN;
        pin_gen_count = 1;
      }
      break;

    case PIN_GENERATE_INIT:
      pin_state = PIN_INPUT_INIT;
      p_type = PROC_PIN_GENERATE;
      pin_gen_count = 1;
      break;

    case PIN_GENERATE_FAIL:
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        pin_state = PIN_GENERATE_INIT;
        *evt_mask &= ~BUTTON_1_EVENT;
      }
      else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        *evt_mask |= PIN_INPUT_CANCEL_EVENT;
        pin_state = PIN_NONE_STATE;
      }
      break;

    case PIN_1ST_GEN_FAIL:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        pin_state = PIN_EMPTY_INIT;
        *evt_mask &= ~BUTTON_2_EVENT;
      }
      break;

    case PIN_GENERATE:
      if(pin_gen_count < 2)
      {
        if((incode.curr_pos+1) < 4)
        {
          pin_state = PIN_SHORT_INPUT_INIT;
        }
        else
        {
          memcpy(&incode2, &incode, sizeof(input_pincode_type));
          
          pin_state = PIN_INPUT_INIT;
          pin_gen_count = 2;
        }
      }
      else
      {
        pin_gen_count = 0;
        if(!memcmp(&incode2, &incode, sizeof(input_pincode_type)))
        {
          pin_state = PIN_GENERATE_OK;
 
          dev_feature.pin_status.pin_len = incode2.curr_pos+1;
          memcpy(dev_feature.pin_status.pin_code, incode2.code, incode2.curr_pos+1);
          dev_feature.pin_status.retry_cnt = 10;
          flash_fd_pin_cached(1);

          // APDU send
          apdu_result = APDU_set_pin_check(APDU_PIN_UPDATE, dev_feature.pin_status.pin_code, dev_feature.pin_status.pin_len, NULL);
          pinValidation = 1;
		  
          pincode_generate_complete(lang);
        }
        else
        {
          if(p_type == PROC_PIN_1ST_GEN)
          {
            pin_state = PIN_1ST_GEN_FAIL;
            pincode_1st_gen_failure(lang);
          }
          else
          {
            pin_state = PIN_GENERATE_FAIL;
            pincode_generate_failure(lang);
          }
        }
      }
      break;

    case PIN_GENERATE_OK:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        pin_state = PIN_NONE_STATE;
        *evt_mask |= PIN_CHECK_OK_EVENT;

      }
      break;

    case PIN_INPUT_INIT:
      input_pin_init(lang);
      break;

    case PIN_SHORT_INPUT_INIT:
      short_input_pin_fail(lang);
      pin_state = PIN_SHORT_INPUT_NOTI;
      break;

    case PIN_SHORT_INPUT_NOTI:
      short_input_pin_noti(evt_mask);
      break;

    case PIN_GEN_1ST_DISP:
      pin_gen_1st_disp(evt_mask);
      break;

    case PIN_GEN_2ND_DISP:
      pin_gen_2nd_disp(evt_mask);
      break;

    case PIN_INPUT_DISP:
      pin_input_disp(lang);
      pin_state = PIN_INPUT;
      break;

    case PIN_INPUT:
      input_pin_code(evt_mask);
      break;

    case PIN_VERIFY_PERM_FAIL:
      *evt_mask = EVENT_NONE;
      // No action...
      break;
	
    default:
      break;
  }
}


void simple_pin_set_owner(pin_check_owner_type owner)
{
  pinowner = owner;
}

void simple_check_pin_init(uint8_t *lang)
{
  memset(&incode, 0x0, sizeof(input_pincode_type));
  pin_input_disp(lang);
}

void simple_check_pin(uint32_t *evt_mask, session_state_type *next_state, uint8_t is_submenu)
{
  uint8_t apdu_result;
  uint8_t retry_cnt;
  
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(incode.curr_pos < CODE_NUMBER)
    pin_number++;
    if(pin_number > 9)pin_number = 0;
    
    screen_disp_underline(incode.curr_pos+1, 0);
    screen_disp_code_num(incode.curr_pos+1, pin_number);
    *evt_mask &= ~BUTTON_1_EVENT;
  
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT) // confirm 1 number
  {
    incode.code[incode.curr_pos] = pin_number;
    screen_disp_code_char(incode.curr_pos+1, '*');
	
    incode.curr_pos++;
    if(incode.curr_pos < CODE_NUMBER)
    {
      pin_number = 0;
      screen_disp_underline(incode.curr_pos+1, 0);
      screen_disp_code_num(incode.curr_pos+1, pin_number);
    }
    else
    {
      incode.curr_pos--;
    }
    
    *evt_mask &= ~BUTTON_2_EVENT;
  
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT) // input complete
  {
    int i;
    incode.code[incode.curr_pos] = pin_number;
    screen_disp_code_char(incode.curr_pos+1, '*');

    for(i=incode.curr_pos;i<CODE_NUMBER;i++)
    {
      screen_disp_underline(i+1, 0);
    }

    apdu_result = APDU_set_pin_check(APDU_PIN_VERIFY, incode.code, incode.curr_pos+1, &retry_cnt);
    if(apdu_result)
    {
      *evt_mask |= PIN_CHECK_OK_EVENT;
    }
    else // mismatch pin !!
    {
      *evt_mask |= PIN_CHECK_NOTOK_EVENT;
    }
	
    *evt_mask &= ~BUTTON_1_2_EVENT;

    if(is_submenu)*next_state = SUB_MENU_FUNC;
    
  }
  else if((*evt_mask & BUTTON_2_LONG_EVENT) == BUTTON_2_LONG_EVENT)  // backspace
  {
    if(incode.curr_pos > 0)
    {
      pin_number = incode.code[incode.curr_pos-1];
      incode.code[incode.curr_pos] = 0;
      screen_disp_code_char(incode.curr_pos+1, ' ');
      screen_disp_underline(incode.curr_pos+1, 1);
      screen_disp_code_num(incode.curr_pos, pin_number);
      incode.curr_pos--;
      
    }
    *evt_mask &= ~BUTTON_2_LONG_EVENT;
  
  }
  
}

