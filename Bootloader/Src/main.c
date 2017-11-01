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
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

#include "screen_disp.h"
#include <stdio.h>
#include <string.h>
#include "msg_dispatch.h"
#include "user_type.h"
#include "res_img.h"
#include "flash_dload.h"

//#define D_DNLOAD_SUPPORT_APDU

/* Private variables ---------------------------------------------------------*/

typedef  void (*pFunction)(void);

pFunction Jump_To_Application;
uint32_t JumpAddress;

#define APPLICATION_ADDRESS     (uint32_t)0x08005000


static hid_raw_msg_type raw_m;

#ifdef D_DNLOAD_SUPPORT_APDU
static hid_data_buff_type hid_apdu_buff;
static hid_data_buff_type temp_apdu_buff;
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

#ifdef D_DNLOAD_SUPPORT_APDU
apdu_rx_callback_t apdu_rx_callback = NULL;
#endif

session_state_type s_state = NONE_STATE;
session_state_type next_state = NONE_STATE;

uint32_t tick_count = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
uint8_t bootloader_enter(void)
{
#if 0
  uint8_t key_1_pressed = 0;
  
  __asm volatile("cpsid i");

  if (HAL_GPIO_ReadPin(USER_KEY1_GPIO_Port, USER_KEY1_Pin) == GPIO_PIN_RESET)
  {
    key_1_pressed = 1;
  }

  __asm volatile("cpsie i");

  return (key_1_pressed);
#else
  uint8_t key_1_pressed = 0;
  uint8_t key_2_pressed = 0;
  
  __asm volatile("cpsid i");

  if (HAL_GPIO_ReadPin(USER_KEY1_GPIO_Port, USER_KEY1_Pin) == GPIO_PIN_RESET)
  {
    key_1_pressed = 1;
  }

  if (HAL_GPIO_ReadPin(USER_KEY2_GPIO_Port, USER_KEY2_Pin) == GPIO_PIN_RESET) 
  {
    key_2_pressed = 1;
  }
  __asm volatile("cpsie i");

  return (key_1_pressed && key_2_pressed);
#endif  
}

uint8_t get_start_addr_data(void)
{
  uint32_t data = 0;
  data = *(__IO uint32_t *)APPLICATION_ADDRESS;

  return (data != 0x00000000);
}

void gpio_key_detect(void)
{
  __asm volatile("cpsid i");

  if (HAL_GPIO_ReadPin(USER_KEY1_GPIO_Port, USER_KEY1_Pin) == GPIO_PIN_RESET)
  {
     // key pressed and not release...
     if(key_data[BTN_GPIO1].status == KEY_RELEASED)
     {
       key_data[BTN_GPIO1].status = KEY_PRESSED;
     }
  }
  else 
  {
     // Key release
     if(key_data[BTN_GPIO1].status == KEY_PRESSED)
     {
       key_data[BTN_GPIO1].status = KEY_RELEASED;
     }
  }

  if (HAL_GPIO_ReadPin(USER_KEY2_GPIO_Port, USER_KEY2_Pin) == GPIO_PIN_RESET) 
  {
     // key pressed and not release...
     if(key_data[BTN_GPIO2].status == KEY_RELEASED)
     {
       key_data[BTN_GPIO2].status = KEY_PRESSED;
     }
  }
  else 
  {
     // Key release
     if(key_data[BTN_GPIO2].status == KEY_PRESSED)
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
  
  if(key_data[BTN_GPIO1].status == KEY_PRESSED) //long key
  {
    key_data[BTN_GPIO1].duration ++;
	
	if(key_data[BTN_GPIO1].duration >= LONG_KEY_PRESS)
    {
      key_data[BTN_GPIO1].duration = 0;
      temp_key_event |= BUTTON_1_LONG_EVENT;
	  key_data[BTN_GPIO1].status = KEY_RELEASED;
    }
  }
  else //short key
  {
    if(key_data[BTN_GPIO1].duration > SHORT_KEY_DURATION)
    {
      key_data[BTN_GPIO1].duration = 0;
      temp_key_event |= BUTTON_1_EVENT;
    }
  }

  if(key_data[BTN_GPIO2].status == KEY_PRESSED) //long key
  {
    key_data[BTN_GPIO2].duration ++;
	
	if(key_data[BTN_GPIO2].duration >= LONG_KEY_PRESS)
    {
      key_data[BTN_GPIO2].duration = 0;
      temp_key_event |= BUTTON_2_LONG_EVENT;
	  key_data[BTN_GPIO2].status = KEY_RELEASED;
    }
  }
  else //short key
  {
    if(key_data[BTN_GPIO2].duration > SHORT_KEY_DURATION)
    {
      key_data[BTN_GPIO2].duration = 0;
      temp_key_event |= BUTTON_2_EVENT;
    }
  }

  do
  {
    if(tick_count % 100 == 0)
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

#ifdef D_DNLOAD_SUPPORT_APDU  
  if(msg_id == APDU_MSG_ID)
  {
#if 0
    if(!pincode_validation())
    {
      memset(raw_m.data, 0x0, sizeof(DATA_MAX_SIZE));
	  raw_m.sub_id = NEED_PIN_VALIFY;
      usb_tx_message((uint8_t *)&raw_m, sizeof(hid_raw_msg_type));
      return;
    }
#endif
	
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
#endif/* D_DNLOAD_SUPPORT_APDU */

  if(msg_id == SET_DNLOAD_MODE)
  {
    __asm volatile("cpsid i");
	boot_mode = BOOT_DNLOAD_MODE;
    event_mask |= SET_DNLOAD_EVENT;
    __asm volatile("cpsie i");
  }
  else if(msg_id == DNLOAD_MSG_ID)
  {
    if(boot_mode != BOOT_DNLOAD_MODE)
    {
      raw_m.sub_id = 0xFD; // invalid boot mode
      send_raw_message(&raw_m);
      return;
    }
    __asm volatile("cpsid i");
    event_mask |= SET_DNLOAD_EVENT;
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

void jump_application_address(void)
{
    /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    { 
      /* Jump to user application */
      JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
      Jump_To_Application = (pFunction) JumpAddress;
      
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
      
      /* Jump to application */
      Jump_To_Application();
    }
    
    while(1){}
}

int main(void)
{
  static uint8_t boot_finish = 0;

  memset(&key_data, 0, sizeof(key_data));

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  //MX_USART1_UART_Init();

#ifdef D_DNLOAD_SUPPORT_APDU
  MX_I2C1_Init();
#endif/* D_DNLOAD_SUPPORT_APDU */

  usb_msg_init();
  screen_disp_init();

#ifdef D_DNLOAD_SUPPORT_APDU  
  APDU_reset();
#endif/* D_DNLOAD_SUPPORT_APDU */

  if(!bootloader_enter() && get_start_addr_data())
  { 
    jump_application_address();
  }
    
  s_state = NONE_STATE;
  event_mask = EVENT_NONE;

  /* Infinite loop */
  while (1)
  {
    next_state = NONE_STATE;

    if((event_mask & SET_DNLOAD_EVENT) == SET_DNLOAD_EVENT)
    {
      event_mask &= ~SET_DNLOAD_EVENT;
      s_state = FLASH_DNLOAD_INIT;
    }
	
    switch(s_state)
    {
      case NONE_STATE:
	  	if(boot_finish == 0)
        {
          next_state = DISP_LOGO;
        }
	  	break;

      case DISP_LOGO:
		
	  	boot_finish = 1;
	  	next_state = MAIN_INIT;
		
	  	break;

      case MAIN_INIT:
        screen_disp_clear();
		screen_disp_bitmap(0, 10, 128, 15, (uint8_t *)firmware_download_128x15);
		next_state = MAIN_IDLE;
	  	break;
    
      case MAIN_IDLE:
        if((event_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
        {
          event_mask &= ~BUTTON_1_EVENT;
        }
        else if((event_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
        {
          event_mask &= ~BUTTON_2_EVENT;
        }
	  	break;

      case FLASH_DNLOAD_INIT:
	  	dnload_msg_dispatch(&raw_m);
		next_state = FLASH_DNLOAD;
	  	break;
		
      case FLASH_DNLOAD:
	  	flash_dnload_state(&event_mask, &raw_m);
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
