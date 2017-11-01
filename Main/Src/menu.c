/**
  ******************************************************************************
  * @file    menu.c
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
  
#include "menu.h"
#include "res_img.h"
#include "screen_disp.h"
#include "pincode.h"
#include "flash_dload.h"
#include "seed_gen.h"
#include "recovery.h"
#include "devconfig.h"
#ifdef D_ADD_PASSPHRASE_IN_SEED
#include "pphrase.h"
#endif/* D_ADD_PASSPHRASE_IN_SEED */
//#include "stm32l0xx_hal.h"

#ifndef NULL
#define NULL 0
#endif

#define NEXT_KEY_POS      0,0,7,4
#define CONFIRM_KEY_POS   116,0,9,7
#define CMENU_POS         20,5,79,16
#define NMENU_POS         60,25,7,6

#define LEVEL_SET(x) meter_level_##x 

static uint8_t brightness_lev = 0;

extern uint8_t APDU_set_factofy_init(void);

typedef struct
{
  uint8_t *lang_str;
  uint8_t checked;
  uint8_t lang;
}lang_sel_type;

static lang_sel_type lang_s[COUN_FONT_MAX];
static uint8_t sel_lang = 0;
static uint8_t disp_inverse = 0;
static uint8_t scrlock_duration = 0;
extern device_feature_type dev_feature;

void step_meter_point(uint8_t step)
{

  if(step == 1)screen_disp_bitmap(12, 27, 100, 6, (uint8_t*)LEVEL_SET(1));
  else if(step == 2)screen_disp_bitmap(12, 27, 100, 6, (uint8_t*)LEVEL_SET(2));
  else if(step == 3)screen_disp_bitmap(12, 27, 100, 6, (uint8_t*)LEVEL_SET(3));
  else if(step == 4)screen_disp_bitmap(12, 27, 100, 6, (uint8_t*)LEVEL_SET(4));
  else if(step == 5)screen_disp_bitmap(12, 27, 100, 6, (uint8_t*)LEVEL_SET(5));

  screen_disp_brightness(step);
}

void menu_disp_brightness(uint8_t lang)
{

  uint8_t *str = NULL;
  screen_disp_clear();

  if(lang == KOR_FONT_E)str = (uint8_t*)h_brightness_79x16;
  else if(lang == ENG_FONT_E)str = (uint8_t*)e_brightness_79x16;
  else if(lang == JAP_FONT_E)str = (uint8_t*)j_brightness_79x16;

  screen_disp_bitmap(24,1,79,16, str);

  screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
  //screen_disp_bitmap(55, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);

  brightness_lev = dev_feature.disp_bright;
  
  step_meter_point(brightness_lev);
}

void menu_disp_inverse(uint8_t lang)
{

  uint8_t *str = NULL;
  disp_inverse = dev_feature.disp_inverse;
  
  screen_disp_clear();

  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_down_7x4);
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  
  if(lang == KOR_FONT_E)str = disp_inverse ? (uint8_t*)h_set_on_30x16 : (uint8_t*)h_set_off_30x16;
  else if(lang == ENG_FONT_E)str = disp_inverse ? (uint8_t*)e_set_on_30x16 : (uint8_t*)e_set_off_30x16;
  else if(lang == JAP_FONT_E)str = disp_inverse ? (uint8_t*)j_set_on_30x16 : (uint8_t*)j_set_off_30x16;

  screen_disp_bitmap(27, 12, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(45, 8, 30, 16, str);
  screen_disp_bitmap(60, 25, 7, 6, (uint8_t*)next_menu_exist_7x6);

}

void menu_disp_scrlock(uint8_t lang)
{
  uint8_t *str = NULL;
  scrlock_duration = dev_feature.scr_lock;
  uint8_t *str_off[COUN_FONT_MAX];
  uint8_t *str_10s[COUN_FONT_MAX];
  uint8_t *str_30s[COUN_FONT_MAX];
  uint8_t *str_1min[COUN_FONT_MAX];

  str_off[KOR_FONT_E] = (uint8_t *)h_lock_off_40x16;
  str_off[ENG_FONT_E] = (uint8_t *)e_lock_off_40x16;
  str_off[JAP_FONT_E] = (uint8_t *)j_lock_off_40x16;

  str_10s[KOR_FONT_E] = (uint8_t *)h_10_sec_40x16;
  str_10s[ENG_FONT_E] = (uint8_t *)e_10_sec_40x16;
  str_10s[JAP_FONT_E] = (uint8_t *)j_10_sec_40x16;

  str_30s[KOR_FONT_E] = (uint8_t *)h_30_sec_40x16;
  str_30s[ENG_FONT_E] = (uint8_t *)e_30_sec_40x16;
  str_30s[JAP_FONT_E] = (uint8_t *)j_30_sec_40x16;

  str_1min[KOR_FONT_E] = (uint8_t *)h_1_min_40x16;
  str_1min[ENG_FONT_E] = (uint8_t *)e_1_min_40x16;
  str_1min[JAP_FONT_E] = (uint8_t *)j_1_min_40x16;

  screen_disp_clear();

  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_down_7x4);
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(scrlock_duration == 1)str = str_off[lang];
  else if(scrlock_duration == 10)str = str_10s[lang];
  else if(scrlock_duration == 30)str = str_30s[lang];
  else if(scrlock_duration == 60)str = str_1min[lang];
  
  screen_disp_bitmap(27, 12, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(45, 8, 40, 16, str);
  screen_disp_bitmap(60, 25, 7, 6, (uint8_t*)next_menu_exist_7x6);
}

void menu_disp_language(uint8_t lang)
{

  uint8_t *str = NULL;
  screen_disp_clear();

  sel_lang = 0;
  
  if(lang == KOR_FONT_E)str = (uint8_t*)h_lang_kor_40x16;
  else if(lang == ENG_FONT_E)str = (uint8_t*)e_lang_eng_40x16;
  else if(lang == JAP_FONT_E)str = (uint8_t*)e_lang_jap_40x16; // TBD..
  	
  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_down_7x4);
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  screen_disp_bitmap(22, 12, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(60, 25, 7, 6, (uint8_t*)next_menu_exist_7x6);
  screen_disp_bitmap(40, 8, 40, 16, str);

  lang_s[0].checked = 1;
  lang_s[0].lang_str = str;
  lang_s[0].lang = lang;

  lang_s[1].checked = 0;
  lang_s[2].checked = 0;

  if(lang == KOR_FONT_E)
  {
    lang_s[1].lang_str = (uint8_t*)h_lang_eng_40x16;
    lang_s[1].lang = ENG_FONT_E;
    lang_s[2].lang_str = (uint8_t*)h_lang_jap_40x16;
    lang_s[2].lang = JAP_FONT_E;
  }
  else if(lang == ENG_FONT_E)
  {
    lang_s[1].lang_str = (uint8_t*)e_lang_kor_40x16;
    lang_s[1].lang = KOR_FONT_E;
    lang_s[2].lang_str = (uint8_t*)e_lang_jap_40x16;  // TBD... changed to "japanese "
    lang_s[2].lang = JAP_FONT_E;
  }
  else if(lang == JAP_FONT_E)
  {
    // TBD...
    
    lang_s[1].lang_str = (uint8_t*)e_lang_kor_40x16;  
    lang_s[1].lang = KOR_FONT_E;
    lang_s[2].lang_str = (uint8_t*)e_lang_eng_40x16;
    lang_s[2].lang = ENG_FONT_E;
  }
}

void menu_disp_model(uint8_t lang)
{
  	
  screen_disp_clear();

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_str(39, 20, "HK-1000", &Font_7x10);

}

void menu_disp_vendor(uint8_t lang)
{
  uint8_t *str = NULL;
  screen_disp_clear();

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(lang == KOR_FONT_E)str = (uint8_t*)h_hyundai_co_ltd_128x20;
  else str = (uint8_t*)e_hyundai_co_ltd_128x20;

  screen_disp_bitmap(0,12,128,20, str);
}

void menu_disp_version(uint8_t lang)
{
  int len;
  uint8_t ver;
  char str_ver[10];
  char app_ver[4];
  char tmp[3];

  ver = (uint8_t)flash_get_sec_extra_ver();

#if !defined ( __CC_ARM )  
	itoa(ver, tmp, 10);
#endif
  len = strlen(tmp);
  if(len > 2)len = 2;
  
  strcpy(app_ver, ".00");
  strcpy(&app_ver[3-len], tmp);

  strcpy(str_ver, BUILD_VER);
  strcat(str_ver, app_ver);
  
  screen_disp_clear();

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_str(10, 5, str_ver, &Font_7x10);
  screen_disp_str(10, 22, BUILD_DATE, &Font_7x10);

}

#ifdef USE_SERIAL_NUM
void menu_disp_serial_num(uint8_t lang)
{
  int len;
  char str_sn[15];
  char tmp[11];
  uint32_t sn = flash_sn_read();
  
#if !defined ( __CC_ARM )  
  itoa(sn, tmp, 10);
#endif
  len = strlen(tmp);
  if(len > 10)len = 10;

  strcpy(str_sn, "0000000000");
  strcpy(&str_sn[10-len], tmp);
  
  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_str(25, 20, str_sn, &Font_7x10);

}
#endif

void menu_disp_factory_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(10,8,"User Data",&Font_7x10);
    screen_disp_str(10,20,"will be erased",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_factory_init_noti_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t*)j_factory_init_noti_90x28;
    screen_disp_bitmap(18,2,90,28,str);
  }
}

void menu_disp_change_pin(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(20,8,"PIN code",&Font_7x10);
	screen_disp_str(20,20,"Please enter",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_curr_pin_input_90x32;
    else if(lang == JAP_FONT_E)str = (uint8_t*)j_curr_pin_input_90x32;
    
    screen_disp_bitmap(19,0,90,32,str);
  }
}

void menu_disp_passphrase(uint8_t lang)
{
  int i;
  uint8_t *str = NULL;
  
  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(lang == ENG_FONT_E)
  {
    if(dev_feature.assign_pphrase == 0)
    {
      screen_disp_str(18,15,"No passphrase",&Font_7x10);
    }
    else
    {
      //for(i=0;i<dev_feature.assign_pphrase && i <= 10; i++)
      //{
      //  screen_disp_char(20+i*10, 20, '*', Font_7x10, 0);
      //}
      screen_disp_str(5,15,"Used in wallet",&Font_7x10);
    }
  }
  else
  {
    if(dev_feature.assign_pphrase == 0)
    {
      if(lang == KOR_FONT_E) str = (uint8_t*)h_not_apply_pphrase_70x16;
      else if(lang == JAP_FONT_E) str = (uint8_t*)h_not_apply_pphrase_70x16; // TBD..
    }
    else
    {
      if(lang == KOR_FONT_E) str = (uint8_t*)h_apply_pphrase_70x16;
      else if(lang == JAP_FONT_E) str = (uint8_t*)h_apply_pphrase_70x16; // TBD...
    }
    screen_disp_bitmap(28,15,70,16,str);
  }
}

void menu_disp_warning_wallet(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(14,8,"Data will be",&Font_7x10);
	screen_disp_str(14,20,"Erase & Update",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_seed_warning100x28;
    else if(lang == JAP_FONT_E)str = (uint8_t*)h_seed_warning100x28; // TBD....
    
    screen_disp_bitmap(14, 2, 100, 28, str);
  }  
}

menu_base_type menu_map[] = {
  {MNU_MAIN_IDLE,        
  {{NULL,}}, 
  {{NULL,}}, 
  {{NULL,}}, 
  {{NULL,}}, 
  {MNU_ID_MAX, MNU_ID_MAX, MNU_ID_MAX},
  {NULL}},

  {MNU_DEVICE_INFO,      
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_device_info_mnu_79x16, (uint8_t*)e_device_info_mnu_79x16, (uint8_t*)e_device_info_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_SETTINGS, MNU_SETTINGS, MNU_MODEL_INFO},
  {NULL}},

  {MNU_SETTINGS,         
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_settings_mnu_79x16, (uint8_t*)e_settings_mnu_79x16, (uint8_t*)e_settings_mnu_79x16},    {CMENU_POS}}, // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_DEVICE_INFO, MNU_DEVICE_INFO, MNU_WALLTE_SETTING},
  {NULL}},

  {MNU_MODEL_INFO,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_model_info_mnu_79x16, (uint8_t*)e_model_info_mnu_79x16, (uint8_t*)e_model_info_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BACK_DEVICE_MENU, MNU_VENDOR_INFO, MNU_FUNC_CALL},
  {menu_disp_model}},

  {MNU_VENDOR_INFO,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_vendor_mnu_79x16, (uint8_t*)e_vendor_mnu_79x16, (uint8_t*)e_vendor_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_MODEL_INFO, MNU_VERSION_INFO, MNU_FUNC_CALL},
  {menu_disp_vendor}},

#ifdef USE_SERIAL_NUM
  {MNU_VERSION_INFO,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_version_mnu_79x16, (uint8_t*)e_version_mnu_79x16, (uint8_t*)e_version_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_VENDOR_INFO, MNU_SERIAL_NUMBER, MNU_FUNC_CALL},
  {menu_disp_version}},

  {MNU_SERIAL_NUMBER,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_serial_mnu_79x16, (uint8_t*)e_serial_mnu_79x16, (uint8_t*)e_serial_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_VERSION_INFO, MNU_BACK_DEVICE_MENU, MNU_FUNC_CALL},
  {menu_disp_serial_num}},

  {MNU_BACK_DEVICE_MENU, 
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_SERIAL_NUMBER, MNU_MODEL_INFO, MNU_DEVICE_INFO},
  {NULL}},
#else  
  {MNU_VERSION_INFO,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_version_mnu_79x16, (uint8_t*)e_version_mnu_79x16, (uint8_t*)e_version_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_VENDOR_INFO, MNU_BACK_DEVICE_MENU, MNU_FUNC_CALL},
  {menu_disp_version}},

  {MNU_BACK_DEVICE_MENU, 
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_VERSION_INFO, MNU_MODEL_INFO, MNU_DEVICE_INFO},
  {NULL}},
#endif

  {MNU_WALLTE_SETTING,      
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_wallet_mnu_79x16, (uint8_t*)e_wallet_mnu_79x16, (uint8_t*)e_wallet_mnu_79x16},    {CMENU_POS}},  // TBD 
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_SETTINGS, MNU_DISPLAY_SETTING, MNU_GENERATE_WALLET},
  {NULL}},
                         
  {MNU_DISPLAY_SETTING,  
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_display_mnu_79x16, (uint8_t*)e_display_mnu_79x16, (uint8_t*)e_display_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_WALLTE_SETTING, MNU_SECURITY, MNU_BRIGHTNESS},
  {NULL}},

  {MNU_SECURITY,         
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_security_mnu_79x16, (uint8_t*)e_security_mnu_79x16, (uint8_t*)e_security_mnu_79x16},    {CMENU_POS}}, // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_DISPLAY_SETTING, MNU_LANGUAGE, MNU_PASSPHRASE},
  {NULL}},

  {MNU_LANGUAGE,         
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_language_mnu_79x16, (uint8_t*)e_language_mnu_79x16, (uint8_t*)e_language_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_SECURITY, MNU_BACK_MAIN_MENU, MNU_FUNC_CALL},
  {menu_disp_language}},
  
  {MNU_BACK_MAIN_MENU,   
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_LANGUAGE, MNU_WALLTE_SETTING, MNU_SETTINGS},
  {NULL}},

  {MNU_GENERATE_WALLET,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_wallet_create_mnu_79x16, (uint8_t*)e_wallet_create_mnu_79x16, (uint8_t*)e_wallet_create_mnu_79x16},    {CMENU_POS}},  // TBD
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BACK_WALLET_MENU, MNU_RECOVERY_WALLET, MNU_FUNC_CALL},
  {menu_disp_warning_wallet}},
  
  {MNU_RECOVERY_WALLET,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_wallet_recovery_mnu_79x16, (uint8_t*)e_wallet_recovery_mnu_79x16, (uint8_t*)e_wallet_recovery_mnu_79x16},    {CMENU_POS}},  // TBD 
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_GENERATE_WALLET, MNU_BACK_WALLET_MENU, MNU_FUNC_CALL},
  {menu_disp_warning_wallet}},

  {MNU_BACK_WALLET_MENU, 
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_RECOVERY_WALLET, MNU_GENERATE_WALLET, MNU_WALLTE_SETTING},
  {NULL}},
  
  {MNU_BRIGHTNESS,       
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_brightness_mnu_79x16, (uint8_t*)e_brightness_mnu_79x16, (uint8_t*)e_brightness_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BACK_DEVICE_MENU, MNU_COLOR_INVERSE, MNU_FUNC_CALL},
  {menu_disp_brightness}},

  {MNU_COLOR_INVERSE,       
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_inverse_mnu_79x16, (uint8_t*)e_inverse_mnu_79x16, (uint8_t*)j_inverse_mnu_79x16},    {CMENU_POS}},
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BRIGHTNESS, MNU_SCREEN_LOCK, MNU_FUNC_CALL},
  {menu_disp_inverse}},

  {MNU_SCREEN_LOCK,       
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_scr_saver_79x16, (uint8_t*)e_scr_saver_79x16, (uint8_t*)j_scr_saver_79x16},    {CMENU_POS}},
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BRIGHTNESS, MNU_BACK_DISP_MENU, MNU_FUNC_CALL},
  {menu_disp_scrlock}},

  {MNU_BACK_DISP_MENU, 
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BRIGHTNESS, MNU_BRIGHTNESS, MNU_DISPLAY_SETTING},
  {NULL}},
  
  {MNU_PASSPHRASE,       
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)e_passphrase_mnu_79x16, (uint8_t*)e_passphrase_mnu_79x16, (uint8_t*)e_passphrase_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_BACK_SEC_MENU, MNU_CHANGE_PIN, MNU_FUNC_CALL},
  {menu_disp_passphrase}},
    
  {MNU_CHANGE_PIN,       
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_change_pin_mnu_79x16, (uint8_t*)e_change_pin_mnu_79x16, (uint8_t*)j_change_pin_mnu_79x16},    {CMENU_POS}},
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_PASSPHRASE, MNU_FACTORY_INIT, MNU_FUNC_CALL},
  {menu_disp_change_pin}},

  {MNU_FACTORY_INIT,     
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_factory_mnu_79x16, (uint8_t*)e_factory_mnu_79x16, (uint8_t*)e_factory_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_CHANGE_PIN, MNU_BACK_SEC_MENU, MNU_FUNC_CALL},
  {menu_disp_factory_init}},

  {MNU_BACK_SEC_MENU,    
  {(uint8_t*)button_down_7x4,       {NEXT_KEY_POS}}, 
  {(uint8_t*)confirm_check_icon,  {CONFIRM_KEY_POS}}, 
  {{(uint8_t*)h_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16, (uint8_t*)e_backmenu_mnu_79x16},    {CMENU_POS}},  // TBD jap
  {(uint8_t*)next_menu_exist_7x6,   {NMENU_POS}},
  {MNU_FACTORY_INIT, MNU_PASSPHRASE, MNU_SECURITY},
  {NULL}},

  {MNU_FUNC_CALL,        
  {{NULL,}}, 
  {{NULL,}}, 
  {{NULL,}}, 
  {{NULL,}}, 
  {MNU_ID_MAX, MNU_ID_MAX, MNU_ID_MAX},
  {NULL}}
	
};


menu_id_type get_next_menu_id(menu_id_type cmenu)
{
  return menu_map[cmenu].link.next_menu;
}

menu_id_type get_prev_menu_id(menu_id_type cmenu)
{
  return menu_map[cmenu].link.prev_menu;
}

menu_id_type get_go_menu_id(menu_id_type cmenu)
{
  return menu_map[cmenu].link.go_menu;
}

void menu_disp_map
(
  uint8_t             *menu_id, 
  language_e_type     *lang
)
{
  if(*menu_id == MNU_ID_MAX)return;
  
  if(menu_map[*menu_id].button1.img_data != NULL)
  screen_disp_bitmap(menu_map[*menu_id].button1.pos.x,
                     menu_map[*menu_id].button1.pos.y,
                     menu_map[*menu_id].button1.pos.width,
                     menu_map[*menu_id].button1.pos.height,
                     menu_map[*menu_id].button1.img_data);

  if(menu_map[*menu_id].button2.img_data != NULL)
  screen_disp_bitmap(menu_map[*menu_id].button2.pos.x,
                     menu_map[*menu_id].button2.pos.y,
                     menu_map[*menu_id].button2.pos.width,
                     menu_map[*menu_id].button2.pos.height,
                     menu_map[*menu_id].button2.img_data);

  if(menu_map[*menu_id].curr_menu_image.str_img[*lang] != NULL)
  screen_disp_bitmap(menu_map[*menu_id].curr_menu_image.pos.x,
                     menu_map[*menu_id].curr_menu_image.pos.y,
                     menu_map[*menu_id].curr_menu_image.pos.width,
                     menu_map[*menu_id].curr_menu_image.pos.height,
                     menu_map[*menu_id].curr_menu_image.str_img[*lang]);

  if(menu_map[*menu_id].next_menu_image.img_data != NULL)
  screen_disp_bitmap(menu_map[*menu_id].next_menu_image.pos.x,
                     menu_map[*menu_id].next_menu_image.pos.y,
                     menu_map[*menu_id].next_menu_image.pos.width,
                     menu_map[*menu_id].next_menu_image.pos.height,
                     menu_map[*menu_id].next_menu_image.img_data);
  
}

void menu_disp_func_call(menu_id_type *menu_id, language_e_type *lang)
{
  uint8_t local_lang = (uint8_t)*lang;
  
  menu_map[*menu_id].disp_func(local_lang);
}

void menu_pin_invalid_disp(uint8_t lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();
  
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(10,15,"PIN is invalid", &Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_invalid_pin_80x13;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_invalid_pin_80x13; // TBD...
    screen_disp_bitmap(24, 10, 80, 13, str);
  }
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void menu_brightness(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    brightness_lev--;
    if(brightness_lev < 1)brightness_lev = 1;
    step_meter_point(brightness_lev);
	
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    brightness_lev++;
    if(brightness_lev > 5)brightness_lev = 5;
    step_meter_point(brightness_lev);
    *evt_mask &= ~BUTTON_2_EVENT;
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    dev_feature.disp_bright = brightness_lev;
    *evt_mask &= ~BUTTON_1_2_EVENT;
    *next_state = MAIN_INIT;

    flash_fd_disp_bright(dev_feature.disp_bright);
  }
}

void menu_inverse(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{

  uint8_t *str = NULL;    

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
    disp_inverse = disp_inverse?0:1;
		
    if(*lang == KOR_FONT_E)str = disp_inverse ? (uint8_t*)h_set_on_30x16 : (uint8_t*)h_set_off_30x16;
    else if(*lang == ENG_FONT_E)str = disp_inverse ? (uint8_t*)e_set_on_30x16 : (uint8_t*)e_set_off_30x16;
    else if(*lang == JAP_FONT_E)str = disp_inverse ? (uint8_t*)j_set_on_30x16 : (uint8_t*)j_set_off_30x16;

    if(dev_feature.disp_inverse == disp_inverse)
    {
      screen_disp_bitmap(27, 12, 9, 7, (uint8_t*)confirm_check_icon);
    }
    else
    {
      screen_disp_bitmap(27, 12, 10, 10, (uint8_t*)rec_10x10_clear);
    }
    screen_disp_bitmap(45, 8, 30, 16, str);
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    dev_feature.disp_inverse = disp_inverse;
    screen_disp_inverse(disp_inverse?1:0);
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;

    flash_fd_disp_inverse(dev_feature.disp_inverse);
  }
}

void menu_language(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
    sel_lang++;
    #ifdef D_ALL_LANGUAGE // Eng + Kor + Jap
    if(sel_lang > (COUN_FONT_MAX-1))sel_lang = 0;
    #else// Eng + other language
    if(sel_lang > (COUN_FONT_MAX-2))sel_lang = 0;
    #endif

    if(lang_s[sel_lang].checked)
    {
      screen_disp_bitmap(22, 12, 9, 7, (uint8_t*)confirm_check_icon);
    }
    else
    {
      screen_disp_bitmap(22, 12, 10, 10, (uint8_t*)rec_10x10_clear);
    }
    screen_disp_bitmap(40, 8, 40, 16, (uint8_t*)lang_s[sel_lang].lang_str);
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *lang = lang_s[sel_lang].lang;
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;

    flash_fd_language(*lang);

  }
}

void menu_model(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
}

void menu_vendor(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
}

void menu_version(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
}

#ifdef USE_SERIAL_NUM
void menu_serial_num(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
}
#endif

void device_factory_init(void)
{
  flash_fd_init();
}

void menu_disp_init_complete(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();  

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(35,8,"Complete", &Font_7x10);
    screen_disp_str(18,20,"Reboot device", &Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_factory_init_complete_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t*)h_factory_init_complete_90x28;  // TBD....
    screen_disp_bitmap(18,2,90,28,str);
  }
}

void menu_factory_init_alert_noti(uint8_t lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
  
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(0,8,"Wallet & All data",&Font_7x10);
    screen_disp_str(0,20,"will be erased",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t*)h_all_data_erase_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t*)h_all_data_erase_90x28;  // TBD....
    screen_disp_bitmap(18,2,90,28,str);
  }
}

void menu_factory_init(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{
  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    *evt_mask &= ~BUTTON_1_2_EVENT;
    *next_state = SIMPLE_PIN_CK_INIT;
  }
  else if((*evt_mask & PIN_CHECK_OK_EVENT) == PIN_CHECK_OK_EVENT)
  {	
    *evt_mask &= ~PIN_CHECK_OK_EVENT;
    *next_state = FAC_INIT_ALERT_NOTI;
    menu_factory_init_alert_noti(*lang);
  }
  else if((*evt_mask & PIN_CHECK_NOTOK_EVENT) == PIN_CHECK_NOTOK_EVENT)
  {
    *evt_mask &= ~PIN_CHECK_NOTOK_EVENT;
    *next_state = SIMPLE_PIN_INVALID;
    menu_pin_invalid_disp(*lang);
  }
}

void menu_factory_init_alert(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{
  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    *evt_mask &= ~BUTTON_1_2_EVENT;
    device_factory_init();

    APDU_set_factofy_init();

    menu_disp_init_complete(*lang);	
    *next_state = FAC_INIT_COMPLETE;
  }
}

void menu_change_pin(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
    *next_state = PIN_CHECK;
    pincode_set_type(PROC_CHANGE_PIN);
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
}

void menu_passphrase(uint32_t *evt_mask, session_state_type *next_state)
{

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
}

void menu_scrlock(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{
  uint8_t *str = NULL;
  uint8_t *str_off[COUN_FONT_MAX];
  uint8_t *str_10s[COUN_FONT_MAX];
  uint8_t *str_30s[COUN_FONT_MAX];
  uint8_t *str_1min[COUN_FONT_MAX];

  str_off[KOR_FONT_E] = (uint8_t *)h_lock_off_40x16;
  str_off[ENG_FONT_E] = (uint8_t *)e_lock_off_40x16;
  str_off[JAP_FONT_E] = (uint8_t *)j_lock_off_40x16;

  str_10s[KOR_FONT_E] = (uint8_t *)h_10_sec_40x16;
  str_10s[ENG_FONT_E] = (uint8_t *)e_10_sec_40x16;
  str_10s[JAP_FONT_E] = (uint8_t *)j_10_sec_40x16;

  str_30s[KOR_FONT_E] = (uint8_t *)h_30_sec_40x16;
  str_30s[ENG_FONT_E] = (uint8_t *)e_30_sec_40x16;
  str_30s[JAP_FONT_E] = (uint8_t *)j_30_sec_40x16;

  str_1min[KOR_FONT_E] = (uint8_t *)h_1_min_40x16;
  str_1min[ENG_FONT_E] = (uint8_t *)e_1_min_40x16;
  str_1min[JAP_FONT_E] = (uint8_t *)j_1_min_40x16;

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
		
    if(scrlock_duration == 1)
    {
      scrlock_duration = 10;
      str = str_10s[*lang];
    }
    else if(scrlock_duration == 10)
    {
      scrlock_duration = 30;
      str = str_30s[*lang];
    }
    else if(scrlock_duration == 30)
    {
      scrlock_duration = 60;
      str = str_1min[*lang];
    }
    else if(scrlock_duration == 60){
      scrlock_duration = 1;
      str = str_off[*lang];
    }

    if(dev_feature.scr_lock == scrlock_duration)
    {
      screen_disp_bitmap(27, 12, 9, 7, (uint8_t*)confirm_check_icon);
    }
    else
    {
      screen_disp_bitmap(27, 12, 10, 10, (uint8_t*)rec_10x10_clear);
    }
    screen_disp_bitmap(45, 8, 40, 16, str);
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    dev_feature.scr_lock = scrlock_duration;
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;

    flash_fd_scr_lock(dev_feature.scr_lock);
  }
}

void menu_recovery_wallet(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
    *next_state = SIMPLE_PIN_CK_INIT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
  else if((*evt_mask & PIN_CHECK_OK_EVENT) == PIN_CHECK_OK_EVENT)
  {
    *evt_mask &= ~PIN_CHECK_OK_EVENT;

#ifdef D_ADD_PASSPHRASE_IN_SEED
    devcfg_set_cfg_mode(1);
    *next_state = PPHRASE_STATE;
    pphrase_set_state(PPHRASE_INIT_S);
#else
    *next_state = SEED_RECOVERY_STATE;
    seed_recovery_set_state(RECOVERY_INIT_S);
#endif	
  }
  else if((*evt_mask & PIN_CHECK_NOTOK_EVENT) == PIN_CHECK_NOTOK_EVENT)
  {
    *evt_mask &= ~PIN_CHECK_NOTOK_EVENT;
    *next_state = SIMPLE_PIN_INVALID;
    menu_pin_invalid_disp(*lang);
  }
}

void menu_generate_wallet(uint32_t *evt_mask, session_state_type *next_state, uint8_t *lang)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    *evt_mask &= ~BUTTON_1_EVENT;
    *next_state = SIMPLE_PIN_CK_INIT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    *next_state = MAIN_INIT;
  }
  else if((*evt_mask & PIN_CHECK_OK_EVENT) == PIN_CHECK_OK_EVENT)
  {
    *evt_mask &= ~PIN_CHECK_OK_EVENT;
#ifdef D_ADD_PASSPHRASE_IN_SEED
    devcfg_set_cfg_mode(0);
    *next_state = PPHRASE_STATE;
    pphrase_set_state(PPHRASE_INIT_S);
#else
    *next_state = SEED_GEN_STATE;
    seed_gen_set_state(SEED_GEN_INIT_S);
#endif	
  }
  else if((*evt_mask & PIN_CHECK_NOTOK_EVENT) == PIN_CHECK_NOTOK_EVENT)
  {
    *evt_mask &= ~PIN_CHECK_NOTOK_EVENT;
    *next_state = SIMPLE_PIN_INVALID;
    menu_pin_invalid_disp(*lang);
  }
}

void menu_submenu_sate
(
  menu_id_type         *menu_id, 
  language_e_type      *lang,
  uint32_t             *evt_mask, 
  session_state_type   *next_state
)
{
  switch(*menu_id)
  {
    case MNU_BRIGHTNESS:
      menu_brightness(evt_mask, next_state);
      break;
    case MNU_COLOR_INVERSE:
      menu_inverse(evt_mask, next_state, lang);
      break;
    case MNU_LANGUAGE:
      menu_language(evt_mask, next_state, lang);
      break;
    case MNU_MODEL_INFO:
      menu_model(evt_mask, next_state);
      break;
    case MNU_VENDOR_INFO:
      menu_vendor(evt_mask, next_state);
      break;
    case MNU_VERSION_INFO:
      menu_version(evt_mask, next_state);
      break;
      
    #ifdef USE_SERIAL_NUM
    case MNU_SERIAL_NUMBER:
      menu_serial_num(evt_mask, next_state);
      break;
    #endif
    
    case MNU_FACTORY_INIT:
      menu_factory_init(evt_mask, next_state, lang);
      break;
    case MNU_CHANGE_PIN:
      menu_change_pin(evt_mask, next_state);
      break;
    case MNU_PASSPHRASE:
      menu_passphrase(evt_mask, next_state);
      break;
    case MNU_SCREEN_LOCK:
      menu_scrlock(evt_mask, next_state, lang);
      break;
    case MNU_RECOVERY_WALLET:
      menu_recovery_wallet(evt_mask, next_state, lang);
      break;
    case MNU_GENERATE_WALLET:
      menu_generate_wallet(evt_mask, next_state, lang);
      break;
    default:
      break;
  }
  
}



