
/**
  ******************************************************************************
  * @file    user_type.h
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

#ifndef __USER_TYPE_H
#define __USER_TYPE_H

#include <stdint.h>
#include "usb_user.h"

#ifndef BUILD_VER
#define BUILD_VER "0"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE "0"
#endif

typedef enum
{
  BOOT_NOMAL_MODE  = 0,
  BOOT_DNLOAD_MODE,
  MAX_BOOT_MODE
}boot_mode_type;

typedef enum 
{
  EVENT_NONE                 = 0x00000000,
  BUTTON_1_EVENT             = 0x00000001,
  BUTTON_1_LONG_EVENT        = 0x00000002,
  BUTTON_2_EVENT             = 0x00000004,
  BUTTON_2_LONG_EVENT        = 0x00000008,
  BUTTON_1_2_EVENT           = 0x00000010,
  BUTTON_1_2_LONG_EVENT      = 0x00000020,
  
  PIN_CHECK_OK_EVENT         = 0x00000040,
  PIN_CHECK_NOTOK_EVENT      = 0x00000080,
  PIN_INPUT_CANCEL_EVENT     = 0x00000100,

  SEED_GENERATED_EVENT       = 0x00000200,
  SEED_RECOVERY_EVENT        = 0x00000400,
  SEED_MAX_RETRY_EVENT       = 0x00000800,

  CONFIRM_PPHRASE_EVENT      = 0x00001000,
  MISMATCH_PPHRASE_EVENT     = 0x00002000,
  CONFIRM_WORD_LIST_EVENT    = 0x00004000,
  MISMATCH_WORD_LIST_EVENT   = 0x00008000,
  
  SEC_UPDATE_START_EVENT     = 0x00100000,
  SEC_UPDATE_STOP_EVENT      = 0x00200000,
  
  // reserved 0x00400000 ~ 0x00800000
  // 2 event
  
  SET_DNLOAD_EVENT           = 0x01000000,
  SET_SCR_SAVER              = 0x02000000,
 
  TEST_EVENT                 = 0x10000000,
  AUTO_TEST_EVENT            = 0x20000000,

  // reserved 0x40000000 ~ 0x80000000
  // 2 event
  
  LOCAL_EVENT_MAX
}local_event_type;

typedef enum
{
  NONE_STATE           = 0x0000,

  DISP_LOGO            = 0x0001,
  PIN_INIT             = 0x0002,
  PIN_CHECK            = 0x0003,
  MAIN_INIT            = 0x0004,
  MAIN_IDLE            = 0x0005,
  SCR_SAVER_INIT       = 0x0006,
  SCR_SAVER            = 0x0007,

  SUB_MENU_FUNC        = 0x0008,
  
  PIN_CODE_STATE       = 0x0010,

  DEV_CONFIGURATION    = 0x0016,


  SEED_GEN_STATE       = 0x0020,
  SEED_RECOVERY_STATE  = 0x0030,
  PPHRASE_STATE        = 0x0040,

  SIMPLE_PIN_CK_INIT   = 0x0100,
  SIMPLE_PIN_CHECK     = 0x0101,
  SIMPLE_PIN_INVALID   = 0x0102,
  FAC_INIT_ALERT_NOTI  = 0x0103,
  FAC_INIT_COMPLETE    = 0x0104,

  SEC_UPDATE_NOTI_INIT = 0x0107,
  SEC_UPDATE_NOTI      = 0x0108,

  FACTORY_TEST         = 0x0200,

  FLASH_DNLOAD_INIT    = 0x1000,
  FLASH_DNLOAD         = 0x1001,
  TEST_STATE           = 0x1FFF,
  STATE_MAX
}session_state_type;

typedef void (*received_raw_msg_callback_t)(uint8_t *msg, uint32_t len);

typedef enum
{
#ifdef D_JAPAN_MODE
  JAP_FONT_E   = 0,
  ENG_FONT_E,
  KOR_FONT_E,
#else
  KOR_FONT_E   = 0,
  ENG_FONT_E,
  JAP_FONT_E,
#endif

  COUN_FONT_MAX
  
}language_e_type;

#define LANG_MAX  COUN_FONT_MAX

typedef struct
{
  uint8_t retry_cnt;
  uint8_t pin_len;
  uint8_t pin_code[10];
}pin_code_s_type;

typedef struct
{
  uint8_t scr_lock;
  uint8_t language;
  uint8_t disp_bright;
  uint8_t disp_inverse;
  uint8_t has_seed;
  pin_code_s_type pin_status;
  uint8_t pin_cached;
  uint8_t assign_pphrase;
  uint8_t boot_hw_test;
}device_feature_type;


#endif/* __USER_TYPE_H */
