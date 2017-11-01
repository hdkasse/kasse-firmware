#ifndef __MENU_H_
#define __MENU_H_
/**
  ******************************************************************************
  * @file    menu.h
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

//#define USE_SERIAL_NUM

typedef struct
{
  uint8_t x;
  uint8_t y;
  uint8_t width;
  uint8_t height;
}disp_position_type;

typedef struct
{
  uint8_t *img_data;
  disp_position_type pos;
}memu_disp_type;

typedef struct
{
  uint8_t *str_img[LANG_MAX];
  disp_position_type pos;
}memu_lang_disp_type;

typedef struct
{
  uint16_t prev_menu;
  uint16_t next_menu;
  uint16_t go_menu;
}memu_linked_type;

typedef enum
{
  MNU_MAIN_IDLE          = 0,
  MNU_DEVICE_INFO,
  MNU_SETTINGS,

  MNU_MODEL_INFO,
  MNU_VENDOR_INFO,
  MNU_VERSION_INFO,
  #ifdef USE_SERIAL_NUM  
  MNU_SERIAL_NUMBER,
  #endif  
  MNU_BACK_DEVICE_MENU,

  MNU_WALLTE_SETTING,
  MNU_DISPLAY_SETTING,
  MNU_SECURITY,
  MNU_LANGUAGE,
  MNU_BACK_MAIN_MENU,

  MNU_GENERATE_WALLET,
  MNU_RECOVERY_WALLET,
  MNU_BACK_WALLET_MENU,
  
  MNU_BRIGHTNESS,
  MNU_COLOR_INVERSE,
  MNU_SCREEN_LOCK,
  MNU_BACK_DISP_MENU,
  
  MNU_PASSPHRASE,
  MNU_CHANGE_PIN,
  MNU_FACTORY_INIT,
  MNU_BACK_SEC_MENU,

  MNU_FUNC_CALL,

  MNU_ID_MAX
}menu_id_type;

//typedef void (*func)(uint32_t *evt_mask);

typedef struct
{
  menu_id_type menu_id;
  
  memu_disp_type button1;
  memu_disp_type button2;

  memu_lang_disp_type curr_menu_image;
  memu_disp_type next_menu_image;

  memu_linked_type link;

  void (*disp_func)(uint8_t lang);
  
}menu_base_type;

menu_id_type get_next_menu_id(menu_id_type cmenu);
menu_id_type get_prev_menu_id(menu_id_type cmenu);
menu_id_type get_go_menu_id(menu_id_type cmenu);

void menu_disp_func_call(menu_id_type *menu_id, language_e_type *lang);

void menu_disp_map
(
  uint8_t             *menu_id, 
  language_e_type     *lang
);

void menu_submenu_sate
(
  menu_id_type         *menu_id, 
  language_e_type      *lang,
  uint32_t             *evt_mask, 
  session_state_type   *next_state
);

void menu_factory_init_alert
(
  uint32_t *evt_mask, 
  session_state_type *next_state, 
  uint8_t *lang
);

#endif/* __MENU_H_ */
