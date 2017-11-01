/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"
#include "i2c.h"
#include "rng.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

#include "screen_disp.h"
#include <stdio.h>
#include <string.h>
#include "msg_dispatch.h"
#include "user_type.h"
#include "pincode.h"
#include "menu.h"
#include "flash_dload.h"
#include "res_img.h"
#include "seed_gen.h"
#include "recovery.h"
#include "devconfig.h"
#include "pphrase.h"
#include "apdu_i2c.h"
#include "trans_noti.h"

/* Private define ------------------------------------------------------------*/

#if 0
#ifdef D_USER_BOOTLOADER
#define APPLICATION_ADDRESS     (uint32_t)0x08005000

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if   (defined ( __CC_ARM ))
//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0348bk/BABGJHDC.html
// below, GNU is not supported
#if 0 // KEIL compiler use .RAMVectorTable section (ref. map file)
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#else
__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#endif
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
__no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#endif
#endif/* D_USER_BOOTLOADER */
#endif

static uint8_t _1_long_key_press = 0;
static uint8_t _2_long_key_press = 0;

static hid_raw_msg_type raw_m;
//static hid_raw_msg_type raw_send;

static hid_data_buff_type hid_apdu_buff;
static hid_data_buff_type temp_apdu_buff;

static menu_id_type curr_menu = MNU_DEVICE_INFO;

device_feature_type dev_feature;
static uint8_t scr_timer_count = 0;
static uint8_t boot_finish = 0;

//#define D_KEY_TEST
#ifdef D_KEY_TEST
extern void key_test(uint32_t *e_mask);
#endif


static uint8_t boot_mode = BOOT_NOMAL_MODE;

#define SHORT_KEY_DURATION 100
#define LONG_KEY_PRESS 1000 //For 2sec when systick 1ms

typedef enum {
  BTN_GPIO1 = 0,
  BTN_GPIO2 = 1,
  BTN_MAX
} button_type;

typedef enum {
  KEY_RELEASED   = 0x0,
  KEY_PRESSED    = 0x1
} key_status_type;

typedef struct {
  key_status_type status;
  uint32_t duration;
} key_data_type;

static key_data_type key_data[BTN_MAX];

uint32_t event_mask = EVENT_NONE;
uint32_t temp_key_event = EVENT_NONE;

#define KEY_EVT_MASK (BUTTON_1_EVENT| \
                      BUTTON_1_LONG_EVENT| \
                      BUTTON_2_EVENT| \
                      BUTTON_2_LONG_EVENT| \
                      BUTTON_1_2_EVENT| \
                      BUTTON_1_2_LONG_EVENT)

received_raw_msg_callback_t rcv_raw_msg_cb = NULL;
apdu_rx_callback_t apdu_rx_callback = NULL;
hid_apdu_callback_t hid_apdu_callback = NULL;


session_state_type s_state = NONE_STATE;
session_state_type next_state = NONE_STATE;
static session_state_type save_curr_state = NONE_STATE;

uint32_t tick_count = 0;
uint32_t hw_test_count = 0;

/* Private function prototypes -----------------------------------------------*/
extern void auto_hw_test(uint32_t *evt_mask, uint8_t test_on);
extern void hw_test_state(uint32_t *evt_mask, session_state_type *next_state);
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void gpio_key_detect(void)
{
  if(!boot_finish)return;
  if(s_state == SEC_UPDATE_NOTI_INIT || s_state == SEC_UPDATE_NOTI) return;
  scr_timer_count = 0;

  __asm volatile("cpsid i");

  if (HAL_GPIO_ReadPin(USER_KEY1_GPIO_Port, USER_KEY1_Pin) == GPIO_PIN_RESET)
  {
     // key pressed and not release...
     //if(key_data[BTN_GPIO1].status == KEY_RELEASED)
     {
       key_data[BTN_GPIO1].status = KEY_PRESSED;
     }
  }
  else 
  {

     // Key release
     //if(key_data[BTN_GPIO1].status == KEY_PRESSED)
     {
       key_data[BTN_GPIO1].status = KEY_RELEASED;
     }
  }

  if (HAL_GPIO_ReadPin(USER_KEY2_GPIO_Port, USER_KEY2_Pin) == GPIO_PIN_RESET) 
  {
     // key pressed and not release...
     //if(key_data[BTN_GPIO2].status == KEY_RELEASED)
     {
       key_data[BTN_GPIO2].status = KEY_PRESSED;
     }
  }
  else 
  {
     // Key release
     //if(key_data[BTN_GPIO2].status == KEY_PRESSED)
     {
       key_data[BTN_GPIO2].status = KEY_RELEASED;
     }
  }
  
  __asm volatile("cpsie i");
}

void key_event_generate(void)
{
  #define SHORT_KEY_1_2 (BUTTON_1_EVENT|BUTTON_2_EVENT)
  #define LONG_KEY_1_2 (BUTTON_1_LONG_EVENT|BUTTON_2_LONG_EVENT)

  tick_count++;
  //__asm volatile("cpsid i");
  
  if(key_data[BTN_GPIO1].status == KEY_PRESSED) //long key
  {
    if(_1_long_key_press == 0)key_data[BTN_GPIO1].duration++;
	
    if(key_data[BTN_GPIO1].duration >= LONG_KEY_PRESS)
    {
      key_data[BTN_GPIO1].duration = 0;
      temp_key_event |= BUTTON_1_LONG_EVENT;
      _1_long_key_press = 1;
      //key_data[BTN_GPIO1].status = KEY_RELEASED;
    }
  }
  else //short key
  {
    if(_1_long_key_press)
    {
      _1_long_key_press = 0;
      key_data[BTN_GPIO1].duration = 0;
    }
    if(key_data[BTN_GPIO1].duration > SHORT_KEY_DURATION)
    {
      key_data[BTN_GPIO1].duration = 0;
      temp_key_event |= BUTTON_1_EVENT;
    }
  }

  if(key_data[BTN_GPIO2].status == KEY_PRESSED) //long key
  {
    if(_2_long_key_press == 0)key_data[BTN_GPIO2].duration++;
	
    if(key_data[BTN_GPIO2].duration >= LONG_KEY_PRESS)
    {
      key_data[BTN_GPIO2].duration = 0;
      temp_key_event |= BUTTON_2_LONG_EVENT;
      _2_long_key_press = 1;
      //key_data[BTN_GPIO2].status = KEY_RELEASED;
    }
  }
  else //short key
  {
    if(_2_long_key_press)
    {
      _2_long_key_press = 0;
      key_data[BTN_GPIO2].duration = 0;
    }
    
    if(key_data[BTN_GPIO2].duration > SHORT_KEY_DURATION)
    {
      key_data[BTN_GPIO2].duration = 0;
      temp_key_event |= BUTTON_2_EVENT;
    }
  }
  //__asm volatile("cpsie i");

  do
  {
    if(tick_count > 100 )
    {
      tick_count = 0;
	  
      if(key_data[BTN_GPIO1].duration > 0 || key_data[BTN_GPIO2].duration > 0)
      {
        break;
      }
  	
      if((temp_key_event & SHORT_KEY_1_2) == SHORT_KEY_1_2)
      {
        __asm volatile("cpsid i");
        event_mask &= ~KEY_EVT_MASK;  // prevent double key recognition
        event_mask |= BUTTON_1_2_EVENT;
        temp_key_event = EVENT_NONE;
        __asm volatile("cpsie i");
      }
      else if((temp_key_event & LONG_KEY_1_2) == LONG_KEY_1_2)
      {
        __asm volatile("cpsid i");
        event_mask &= ~KEY_EVT_MASK; // prevent double key recognition
        event_mask |= BUTTON_1_2_LONG_EVENT;
        temp_key_event = EVENT_NONE;
        __asm volatile("cpsie i");
      }
      else if(temp_key_event != EVENT_NONE)
      {
        __asm volatile("cpsid i");
        event_mask &= ~KEY_EVT_MASK; // prevent double key recognition
        event_mask |= temp_key_event;
        temp_key_event = EVENT_NONE;
        __asm volatile("cpsie i");
      }
    }
  }while(0);

}

/*
   USB Rx Message Format

   
   |  1 Byte  |  1 Byte      | 2 Byte       |       Max 56 Byte       | 
   |  Msg ID  |  sub ID      | Data length  |       Data              |
*/
void raw_message_callback(uint8_t *msg, uint32_t len)
{
  uint32_t msg_id = 0;

  msg_id = raw_message_dispatch(msg, &raw_m);

  if(msg_id == NONE_MSG_ID)return;

  if(msg_id == GET_VERSION_ID)
  {
    get_device_sw_version(&raw_m);
    return;
  }

  if(msg_id == GET_DEV_SN || msg_id == PUT_DEV_SN)
  {
    handle_device_sn(&raw_m, msg_id);
    return;
  }
  
  if(msg_id == APDU_MSG_ID)
  {
    if(!pincode_validation())
    {
      memset(raw_m.data, 0x0, sizeof(DATA_MAX_SIZE));
      raw_m.sub_id = NEED_PIN_VERIFY;
      usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
      return;
    }
	
    if(apdu_msg_prepare(&temp_apdu_buff, &raw_m))
    {
      memcpy(&hid_apdu_buff, &temp_apdu_buff, sizeof(hid_data_buff_type));
      memset(&temp_apdu_buff, 0x0, sizeof(hid_data_buff_type));
      memset(raw_m.data, 0x0, DATA_MAX_SIZE);

      if(apdu_rx_callback != NULL)
      {
        apdu_rx_callback((void*)&hid_apdu_buff);
      }
    }
    else /* response middle step */
    {
      memset(raw_m.data, 0x0, DATA_MAX_SIZE);
      raw_m.data_len = 0;
      usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
    }
  }
  if(msg_id == A_UPDATE_ID)
  {
    if(hid_apdu_callback != NULL)
    {
      hid_apdu_callback((void*)&raw_m);
    }
  }
  if(msg_id == SET_DNLOAD_MODE || msg_id == DNLOAD_MSG_ID)
  {
    memset(raw_m.data, 0x0, DATA_MAX_SIZE);
    raw_m.data_len = 0;
    raw_m.msg_id = msg_id;
    raw_m.sub_id = INVALID_BOOT_MODE;
    usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
  }

  if(msg_id == SEC_UPDATE_ID)
  {
    if(!pincode_validation())
    {
      memset(raw_m.data, 0x0, sizeof(DATA_MAX_SIZE));
      raw_m.sub_id = NEED_PIN_VERIFY;
      usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
      return;
    }  

    if(raw_m.sub_id == UPDATE_START)
    {
      __asm volatile("cpsid i");
      scr_timer_count = 0;
      event_mask |= SEC_UPDATE_START_EVENT;
      __asm volatile("cpsie i");
    }
    else if(raw_m.sub_id == UPDATE_STOP)
    {
      __asm volatile("cpsid i");
      scr_timer_count = 0;
      event_mask |= SEC_UPDATE_STOP_EVENT;
      __asm volatile("cpsie i");
    }
    usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
	
  }
  
  if(msg_id == HW_TEST_ID)
  {
    if((s_state == SEED_GEN_STATE) ||(s_state == SEED_RECOVERY_STATE))//||(s_state == PIN_CODE_STATE))
    {
      memset(raw_m.data, 0x0, sizeof(DATA_MAX_SIZE));
      raw_m.msg_id = msg_id;
      raw_m.data_len = 1;
      raw_m.data[0] = 2;
      usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
      return;
    }
    __asm volatile("cpsid i");
    event_mask |= TEST_EVENT;
    __asm volatile("cpsie i");
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  gpio_key_detect();
}

void HAL_SYSTICK_Callback(void)
{  
  key_event_generate();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{  
  if(htim->Instance == TIM2)
  {
    if(s_state == SEC_UPDATE_NOTI_INIT || s_state == SEC_UPDATE_NOTI)
    {
      return;
    }
    
    if(dev_feature.scr_lock == 1)return;
#ifdef D_BOOT_HW_TEST
    if(s_state == FACTORY_TEST) return;
#endif
    if((++scr_timer_count % dev_feature.scr_lock)==0)
    {
        scr_timer_count = 0;
    }
    else
    {
      return;
    }
	
    if(!boot_finish)return;
    if(s_state == SCR_SAVER)return;
    if(s_state == TEST_STATE) return;
    __asm volatile("cpsid i");
    event_mask |= SET_SCR_SAVER;
    __asm volatile("cpsie i");
  }
}

void main_retrieve_dev_feature(void)
{   
   // if needed, APDU send/resp here !!
   

}

int main(void)
{
  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

#if 0
#ifdef D_USER_BOOTLOADER
  /* Copy the vector table from the Flash (mapped at the base of the application
     load address 0x08005000) to the base address of the SRAM at 0x20000000. */
  for(int i = 0; i < 48; i++)
  {
    VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
  }
#endif/* D_USER_BOOTLOADER */
#endif

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  //MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_RNG_Init();
  MX_TIM2_Init();
  MX_RTC_Init();

  usb_msg_init();
  screen_disp_init();

  flash_fd_read(&dev_feature);
  screen_disp_inverse(dev_feature.disp_inverse);
  screen_disp_brightness(dev_feature.disp_bright);
  
  APDU_reset();

  HAL_TIM_Base_Start_IT(&htim2); //For using TIM2 INTR

  memset(&key_data, 0, sizeof(key_data));
  s_state = NONE_STATE;
  event_mask = EVENT_NONE;

#ifdef D_KEY_TEST
  s_state = TEST_STATE;
  boot_finish = 1;
#endif

  /* Infinite loop */
  while (1)
  {
    next_state = NONE_STATE;

    if((event_mask & SET_SCR_SAVER) == SET_SCR_SAVER)
    {
      event_mask &= ~SET_SCR_SAVER;
      save_curr_state = s_state;
      s_state = SCR_SAVER_INIT;
    }

    if((event_mask & SEC_UPDATE_START_EVENT) == SEC_UPDATE_START_EVENT)
    {
      if((s_state != SCR_SAVER) && (s_state != SCR_SAVER_INIT))
      {
        if((event_mask & SEC_UPDATE_START_EVENT) == SEC_UPDATE_START_EVENT)
        {
          event_mask &= ~SEC_UPDATE_START_EVENT;
          s_state = SEC_UPDATE_NOTI_INIT;
        }
      }
    }
    
    if((event_mask & TEST_EVENT) == TEST_EVENT)
    {
      //event_mask &= ~TEST_EVENT;
      if((s_state == SCR_SAVER) ||(s_state == SCR_SAVER_INIT))
      {
        screen_disp_stop_scroll();
        screen_disp_clear();
        screen_disp_restore_screen();
      }
      else if(s_state == DISP_LOGO)
      {
        screen_disp_stop_scroll();
        screen_disp_backup_screen();
        screen_disp_clear();
        save_curr_state = s_state;
      }
      else
      {
        save_curr_state = s_state;
      }
      hw_test_count ++;
	  s_state = TEST_STATE;
    }		
	
    switch(s_state)
    {
      case NONE_STATE:
        if(boot_finish == 0)
        {
          screen_disp_clear();
#ifdef D_BOOT_HW_TEST
          if(dev_feature.boot_hw_test != 8)
          {
            next_state = FACTORY_TEST;
          }
          else
          {
#endif
            screen_disp_bitmap(0,0,128,32,(uint8_t *)kasse_log_128x32);
		    HAL_Delay(1000);
		    screen_disp_horizon_scroll_left(0, 3, 0, 0x7f,0);
            next_state = DISP_LOGO;
#ifdef D_BOOT_HW_TEST
          }
#endif
          boot_finish = 1;
        }
        break;

      case FACTORY_TEST:
        hw_test_state(&event_mask, &next_state);
        break;

      case SCR_SAVER_INIT:
        screen_disp_backup_screen();
        screen_disp_clear();
        screen_disp_bitmap(0,0,128,32,(uint8_t *)kasse_log_128x32);		  
        screen_disp_horizon_scroll_left(0, 3, 0, 0x7f,0);
        next_state = SCR_SAVER;
        break;

      case SCR_SAVER:
        if(((event_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
           || ((event_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
           || ((event_mask & SEC_UPDATE_START_EVENT) == SEC_UPDATE_START_EVENT))
        {
          screen_disp_stop_scroll();
          screen_disp_clear();
          screen_disp_restore_screen();
          __asm volatile("cpsid i");

          if(((event_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
           || ((event_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT))
          {
            event_mask = EVENT_NONE;
          }
          
          if(save_curr_state == DISP_LOGO)
          {
            next_state = PIN_INIT;
          }
          else
          {
            next_state = save_curr_state;
          }
		  
          save_curr_state = NONE_STATE;
          __asm volatile("cpsie i");
        }
        break;

      case DISP_LOGO:
        if(((event_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
          || ((event_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT))
        {
          screen_disp_stop_scroll();
          event_mask = EVENT_NONE;
      	  next_state = PIN_INIT;
        }
        else if((event_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
        {
          auto_hw_test(&event_mask, 0); //For aging test
        }
        break;

      case PIN_INIT:
        main_retrieve_dev_feature();
        next_state = PIN_CHECK;
        pincode_set_type(PROC_PIN_VERIFY);
        break;

      case PIN_CHECK:
        if(pincode_is_cached())
        {
          pincode_set_state(PIN_VERIFY_INIT);
        }
        else
        {
          pincode_set_state(PIN_EMPTY_INIT);
        }
        next_state = PIN_CODE_STATE;
        break;

      case PIN_CODE_STATE:
        pin_state_mach(&event_mask, &dev_feature.language);
        if((event_mask & PIN_CHECK_OK_EVENT) == PIN_CHECK_OK_EVENT)
        {
          event_mask &= ~PIN_CHECK_OK_EVENT;
    	  
          if(pincode_get_type() == PROC_CHANGE_PIN)
          {
            pincode_set_state(PIN_GENERATE_INIT);
          }
          else
          {
            next_state = MAIN_INIT;
          }
        }
        else if((event_mask & PIN_CHECK_NOTOK_EVENT) == PIN_CHECK_NOTOK_EVENT)
        {
          event_mask &= ~PIN_CHECK_NOTOK_EVENT;
          if(pincode_get_type() == PROC_CHANGE_PIN)
          {
            next_state = MAIN_INIT;
          }
          else
          {
            next_state = PIN_CHECK;
          }
    
        }
        else if((event_mask & PIN_INPUT_CANCEL_EVENT) == PIN_INPUT_CANCEL_EVENT)
        {
          event_mask &= ~PIN_INPUT_CANCEL_EVENT;
          next_state = MAIN_INIT;
        }
        break;

      case SEED_GEN_STATE:
        seed_gen_state(&event_mask, &dev_feature.language);
        if((event_mask & SEED_GENERATED_EVENT) == SEED_GENERATED_EVENT)
        {
          event_mask &= ~SEED_GENERATED_EVENT;
          next_state = MAIN_INIT;
        }
        else if((event_mask & SEED_MAX_RETRY_EVENT) == SEED_MAX_RETRY_EVENT)
        {
          event_mask &= ~SEED_MAX_RETRY_EVENT;
          next_state = MAIN_INIT;
        }
	  	break;
		
      case SEED_RECOVERY_STATE:
        seed_recovery_state(&event_mask, &dev_feature.language);
        if((event_mask & SEED_RECOVERY_EVENT) == SEED_RECOVERY_EVENT)
        {
          event_mask &= ~SEED_RECOVERY_EVENT;
          next_state = MAIN_INIT;
        }
        break;

      case PPHRASE_STATE:
        pphrase_state(&event_mask, &dev_feature.language, &next_state);
        break;

      case DEV_CONFIGURATION:
        devcfg_state(&event_mask, &dev_feature.language, &next_state);
        break;

      case SEC_UPDATE_NOTI_INIT:
        sec_update_notify_init(dev_feature.language);
        next_state = SEC_UPDATE_NOTI;
        break;

      case SEC_UPDATE_NOTI:
        if((event_mask & SEC_UPDATE_STOP_EVENT) == SEC_UPDATE_STOP_EVENT)
        {
          event_mask &= ~SEC_UPDATE_STOP_EVENT;
          curr_menu = MNU_DEVICE_INFO;
          next_state = MAIN_INIT;
        }
        else
        {
          event_mask = EVENT_NONE;
        }
        break;

      case MAIN_INIT:
        #ifdef D_TEST_ONLY
        if(1)
        {
          dev_feature.has_seed = 1;
        }
        #endif/* D_TEST_ONLY */
		
        if(dev_feature.has_seed)
        {
          screen_disp_clear();
          menu_disp_map(&curr_menu, &dev_feature.language);
          next_state = MAIN_IDLE;
        }
        else
        {
          next_state = DEV_CONFIGURATION;
          devcfg_set_state(DEVCFG_INIT_S);
        }
        break;
    
      case MAIN_IDLE:
        if((event_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
        {
          screen_disp_clear();
          event_mask &= ~BUTTON_1_EVENT;
      	  curr_menu = get_next_menu_id(curr_menu);
          menu_disp_map(&curr_menu, &dev_feature.language);
        }
        else if((event_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
        {
          screen_disp_clear();
          event_mask &= ~BUTTON_2_EVENT;
          if(get_go_menu_id(curr_menu) == MNU_FUNC_CALL)
          {
            menu_disp_func_call(&curr_menu, &dev_feature.language);
            next_state = SUB_MENU_FUNC;
          }
          else
          {
            curr_menu = get_go_menu_id(curr_menu);
            menu_disp_map(&curr_menu, &dev_feature.language);
          }
        }
        break;

      case SUB_MENU_FUNC:
        menu_submenu_sate(&curr_menu, &dev_feature.language, &event_mask, &next_state);
        break;

      case SIMPLE_PIN_CK_INIT:
        simple_check_pin_init(&dev_feature.language);
        next_state = SIMPLE_PIN_CHECK;
        break;

      case SIMPLE_PIN_CHECK:
        simple_check_pin(&event_mask, &next_state, 1);
        break;

      case SIMPLE_PIN_INVALID:
        if((event_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
        {
          event_mask &= ~BUTTON_2_EVENT;
          next_state = MAIN_INIT;
        }
        break;

      case FAC_INIT_ALERT_NOTI:
        menu_factory_init_alert(&event_mask, &next_state, &dev_feature.language);
        break;
        
      case FAC_INIT_COMPLETE:
        // no action
        break;

      case TEST_STATE:
#ifdef D_KEY_TEST
        key_test(&event_mask);
#endif
        if((event_mask & TEST_EVENT) == TEST_EVENT)
        {
          uint8_t ret=0;
          extern uint8_t default_hw_test(uint16_t test_num);
          event_mask &= ~TEST_EVENT;
          if(save_curr_state != DISP_LOGO)
          {
            screen_disp_backup_screen();
          }
          screen_disp_clear();
          ret = default_hw_test(hw_test_count);
          screen_disp_restore_screen();
          raw_m.data[0] = ret;
          raw_m.data[1] = hw_test_count;
          __asm volatile("cpsid i");
          usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
          event_mask = EVENT_NONE;
          next_state = save_curr_state;
          if(next_state == DISP_LOGO)
          {
            screen_disp_horizon_scroll_left(0, 3, 0, 0x7f,0);
          }
          save_curr_state = NONE_STATE;
          __asm volatile("cpsie i");
        }
        break;
		
      default:
        break;
    }

    if(next_state != NONE_STATE)
    {
      s_state = next_state;
    }

  }

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
