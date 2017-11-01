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
  
  PIN_CHECK_OK_EVENT         = 0x00000200,
  PIN_CHECK_NOTOK_EVENT      = 0x00000400,
  PIN_INPUT_CANCEL_EVENT     = 0x00000800,

  SEED_GENERATED_EVENT       = 0x00001000,
  SEED_RECOVERY_EVENT        = 0x00002000,
  
  SET_DNLOAD_EVENT           = 0x01000000,
  SET_SCR_SAVER              = 0x02000000,
 
  TEST_EVENT                 = 0x10000000,
  AUTO_TEST_EVENT            = 0x20000000,
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
  FAC_INIT_COMPLETE    = 0x0103,

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
  uint8_t invalid_cnt;
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
  uint8_t boot_hw_test;
}device_feature_type;


#endif/* __USER_TYPE_H */
