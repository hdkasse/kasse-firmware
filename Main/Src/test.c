/**
  ******************************************************************************
  * @file    test.c
  * @author  Hyundai-pay Blockchain IOT Lab
  *          by jyhwang@hyundai-pay.com
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

#include "user_type.h"
#include "screen_disp.h"
#include "res_img.h"
#include "apdu_i2c.h"
#include "flash_dload.h"

#define FEATURE_SET_APP_PROC
#define HW_TEST_START 5
static uint8_t auto_test_flag=0;

typedef enum {
  HW_TEST_NONE = 0,
  HW_TEST_INIT_STATE,
  HW_TEST_OLED_STATE,
  HW_TEST_OLED2_STATE,
  HW_TEST_OLED3_STATE,
  HW_TEST_APDU_STATE,
  HW_TEST_APDU2_STATE,
  HW_TEST_FAIL_STATE,
  HW_TEST_END_STATE,
  HW_TES_MAX
} hw_test_state_type;
static hw_test_state_type test_state = HW_TEST_INIT_STATE;
static uint8_t saved_test = 0;
extern device_feature_type dev_feature;

void key_test(uint32_t *e_mask)
{
  if((*e_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    screen_clear(BLACK);
    screen_disp_str(0, 20, "B_1", &Font_7x10);
    *e_mask &= ~BUTTON_1_EVENT;
  }
  else if((*e_mask & BUTTON_1_LONG_EVENT) == BUTTON_1_LONG_EVENT)
  {
    screen_clear(BLACK);
    screen_disp_str(0, 20, "B_1_LONG", &Font_7x10);
    *e_mask &= ~BUTTON_1_LONG_EVENT;
  }
  else if((*e_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    screen_clear(BLACK);
    screen_disp_str(0, 20, "B_2", &Font_7x10);
    *e_mask &= ~BUTTON_2_EVENT;
  }
  else if((*e_mask & BUTTON_2_LONG_EVENT) == BUTTON_2_LONG_EVENT)
  {
    screen_clear(BLACK);
    screen_disp_str(0, 20, "B_2_LONG", &Font_7x10);
    *e_mask &= ~BUTTON_2_LONG_EVENT;
  }
  else if((*e_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    screen_clear(BLACK);
    screen_disp_str(0, 20, "B_1_2", &Font_7x10);
    *e_mask &= ~BUTTON_1_2_EVENT;
  }
  else if((*e_mask & BUTTON_1_2_LONG_EVENT) == BUTTON_1_2_LONG_EVENT)
  {
    screen_clear(BLACK);
    screen_disp_str(0, 20, "B_1_2_LONG", &Font_7x10);
    *e_mask &= ~BUTTON_1_2_LONG_EVENT;
  }
  else
  {
    //
  }
}

void screen_test(void)
{
  screen_disp_verify_all_page();
  screen_disp_verify_page(PAGE0, PAGE_END);
  screen_clear(BLACK);
  screen_disp_bitmap(0,0,128,32,(uint8_t *)hpay_kasse_logo);
  //screen_disp_horizon_scroll_left(0, 3, 0, 0x7f,1);
  //screen_disp_horizon_scroll_right(0, 3, 0, 0x7f,1);
  OLED_DELAY(500);
  screen_clear(BLACK);
}

uint8_t default_hw_test(uint32_t test_num)
{
  uint8_t result = 0;
  char buffer[10] = {0,};

  if(test_num != 0)
  {
#if !defined ( __CC_ARM )
    itoa(test_num, buffer, 10);
#endif
  }
  screen_clear(BLACK);
  screen_disp_str(0, 0, "HW test Enter", &Font_7x10);
  HAL_Delay(1000);
  screen_test();
  screen_disp_str(0, 0, "OLED test OK", &Font_7x10);
  HAL_Delay(1000);
  screen_disp_str(0, 11, "APDU Test -> ", &Font_7x10);
  result = APDU_command_test();
  HAL_Delay(500);
  screen_disp_str(90, 11, (result==1?"Pass":"Fail"), &Font_7x10);
  HAL_Delay(500);
  screen_disp_str(0, 22, "HW Test End", &Font_7x10);
  if(test_num != 0)
  {
    screen_disp_str(82, 22, buffer, &Font_7x10);
  }
  HAL_Delay(2000);

  return result;
}

void auto_hw_test(uint32_t *evt_mask, uint8_t test_on)
{
  static uint8_t auto_test_start=0;
  uint32_t i=0;

  if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    *evt_mask &= ~BUTTON_1_2_EVENT;
    if(++auto_test_flag == HW_TEST_START)
    {
      auto_test_start ^= 1;
      if(auto_test_start)
      {
        screen_disp_stop_scroll();
        screen_disp_backup_screen();

        while(1)
        {
          i++;
          default_hw_test(i);
          HAL_Delay(100);
        }
      }

      if(!auto_test_start)
      {
        screen_disp_restore_screen();
        screen_disp_horizon_scroll_left(0, 3, 0, 0x7f,0);
      }

      auto_test_flag = 0;
    }
  }
}

void hw_test_state(uint32_t *evt_mask, session_state_type *next_state)
{
  uint8_t result = 0;

  switch(test_state)
  {
    case HW_TEST_NONE:
    break;
    
    case HW_TEST_INIT_STATE:
      screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
      screen_disp_bitmap(8, 0, 9, 7, (uint8_t*)confirm_check_icon);
      screen_disp_str(0, 11, "HW test Enter", &Font_7x10);
      if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
      {
        *evt_mask &= ~BUTTON_1_2_EVENT;
        screen_clear(BLACK);
        if((dev_feature.boot_hw_test == 0) ||(dev_feature.boot_hw_test == 1))// ||(dev_feature.boot_hw_test == 8))
        {
          screen_disp_str(0, 11, "Go to OLED test", &Font_7x10);
          test_state = HW_TEST_OLED_STATE;
        }
#ifdef FEATURE_SET_APP_PROC
        else if((dev_feature.boot_hw_test == HW_TEST_APDU_STATE) || (dev_feature.boot_hw_test == HW_TEST_APDU2_STATE))
#else
        else if(dev_feature.boot_hw_test == HW_TEST_APDU_STATE)
#endif
        {
          saved_test = dev_feature.boot_hw_test;
          screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
          screen_disp_str(0, 11, (saved_test==HW_TEST_APDU_STATE?"APDU test1 Failed":"APDU test2 Failed"), &Font_7x10);
          test_state = (hw_test_state_type)saved_test;
        }
      }
    break;

    case HW_TEST_OLED_STATE:
      screen_disp_bitmap(8, 0, 9, 7, (uint8_t*)confirm_check_icon);
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
        screen_clear(WHITE);
        test_state = HW_TEST_OLED2_STATE;
      }
    break;

    case HW_TEST_OLED2_STATE:
      if(((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT) || ((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT))
      {
        if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
        {
          *evt_mask &= ~BUTTON_1_EVENT;
        }
        else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
        {
          *evt_mask &= ~BUTTON_2_EVENT;
        }
        screen_clear(BLACK);
        test_state = HW_TEST_OLED3_STATE;
      }
    break;

    case HW_TEST_OLED3_STATE:
      screen_disp_bitmap(8, 0, 9, 7, (uint8_t*)confirm_check_icon);
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
        screen_clear(BLACK);
        screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
        screen_disp_str(0, 11, "APDU Test1-> ", &Font_7x10);
        test_state = HW_TEST_APDU_STATE;
      }
    break;

    case HW_TEST_APDU_STATE:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        screen_disp_str(0, 11, "APDU Test1->     ", &Font_7x10);
         result = APDU_command_test();
         HAL_Delay(500);
         screen_disp_str(90, 11, (result==1?"Pass":"Fail"), &Font_7x10);
        if(result==1)
        {
#ifdef FEATURE_SET_APP_PROC
          test_state = HW_TEST_APDU2_STATE;
#else
          test_state = HW_TEST_END_STATE;
#endif
        }
        else
        {
          saved_test = (uint8_t)test_state;
          dev_feature.boot_hw_test = saved_test;
          flash_fd_boot_hw_test(dev_feature.boot_hw_test);
          test_state = HW_TEST_FAIL_STATE;
        }
      }
    break;

    case HW_TEST_APDU2_STATE:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        screen_disp_str(0, 22, "APDU Test2-> ", &Font_7x10);
        HAL_Delay(1000);
        *evt_mask &= ~BUTTON_2_EVENT;
        if(result == 7)
        {
          screen_disp_clear();
          test_state = HW_TEST_END_STATE;
        }
        else
        {
          saved_test = (uint8_t)test_state;
          dev_feature.boot_hw_test = saved_test;
          flash_fd_boot_hw_test(dev_feature.boot_hw_test);
          test_state = HW_TEST_FAIL_STATE;
        }
      }
      break;

    case HW_TEST_FAIL_STATE:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        screen_disp_clear();
        screen_disp_str(0, 11, (saved_test==HW_TEST_APDU_STATE?"APDU test1 Failed":"APDU test2 Failed"), &Font_7x10);
        screen_disp_str(0, 22, "Inform to R&D Team !!", &Font_7x10);
      }
    break;

    case HW_TEST_END_STATE: 
      screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
      screen_disp_bitmap(8, 0, 9, 7, (uint8_t*)confirm_check_icon);
      screen_disp_str(0, 22, "HW Test End", &Font_7x10);
      if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
      {
        *evt_mask &= ~BUTTON_1_2_EVENT;
        dev_feature.boot_hw_test = (uint8_t)test_state;
        flash_fd_boot_hw_test(dev_feature.boot_hw_test);
        test_state = HW_TEST_NONE;
        screen_clear(BLACK);
        screen_disp_bitmap(0,0,128,32,(uint8_t *)kasse_log_128x32);
		 HAL_Delay(500);
		 screen_disp_horizon_scroll_left(0, 3, 0, 0x7f,0);
		 *next_state = DISP_LOGO;
	   }
    break;

    default:
    break;
  }
}
