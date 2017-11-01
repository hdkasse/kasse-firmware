/**
  ******************************************************************************
  * @file    devconfig.c
  * @author  Hyundai-pay Blockchain IOT Lab
  *          by nakjink@hyundai-pay.com
  ******************************************************************************
  *
  * Permission is hereby granted, free of charge, to any person obtaining
  * a copy of this software and associated documentation files (the "Software"),
  * to deal in the Software without restriction, including without limitation
  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the
  * Software is furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included
  * in all copies or substantial portions of the Software.
  *
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

#include "screen_disp.h"
#include "res_img.h"
#include "user_type.h"
#include "devconfig.h"
#include "seed_gen.h"
#include "recovery.h"

#ifdef D_ADD_PASSPHRASE_IN_SEED
#include "pphrase.h"
#endif/* D_ADD_PASSPHRASE_IN_SEED */

static devcfg_state_type devcfg_s = DEVCFG_NONE_S;
static uint8_t sel_cfg_mode = 0;

void devcfg_set_cfg_mode(uint8_t cfg_mode)
{
  sel_cfg_mode = cfg_mode;
}

uint8_t devcfg_get_cfg_mode(void)
{
  return sel_cfg_mode;
}

void devcfg_set_state(devcfg_state_type state)
{
  devcfg_s = state;
}

void devcfg_init_disp(uint8_t lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(4,8,"Configure Wallet",&Font_7x10);
    screen_disp_str(4,20,"Recover or Create",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_device_cfg_120x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_device_cfg_120x28; // TBD....
    
    screen_disp_bitmap(0, 4, 120, 28, str);
  }

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

}

void devcfg_init_disp_event(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    devcfg_s = DEVCFG_START_NOTI_INIT_S;
  }
}

void devcfg_start_noti_init(uint8_t lang)
{
  uint8_t *str = NULL;

  sel_cfg_mode = 0;

  screen_disp_clear();
  
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(1,8,"Press both buttons",&Font_7x10);
    screen_disp_str(1,20,"to the next step",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_cfg_start_120x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_cfg_start_120x28; // TBD....

    screen_disp_bitmap(4, 2, 120, 28, str);
  }

}

void devcfg_start_noti_event(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    *evt_mask &= ~BUTTON_1_2_EVENT;
    devcfg_s = DEVCFG_SEL_CFG_INIT_S;
  }
}

void devcfg_select_cfg_init(uint8_t lang)
{
  uint8_t *str = NULL;
  
  if(lang == KOR_FONT_E)str = (uint8_t *)h_sel_create_79x13;
  else if(lang == ENG_FONT_E)str = (uint8_t *)e_sel_create_79x13;
  else if(lang == JAP_FONT_E)str = (uint8_t *)h_sel_create_79x13; // TBD....

  screen_disp_clear();
  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_down_7x4);
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(60, 25, 7, 6, (uint8_t*)next_menu_exist_7x6);
  screen_disp_bitmap(24, 10, 79, 13, str);
  
}

void devcfg_select_cfg_event(uint32_t *evt_mask, uint8_t lang, session_state_type *next_state)
{
  uint8_t *str = NULL;

  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    sel_cfg_mode = ~sel_cfg_mode;
    
    if(sel_cfg_mode)
    {
      if(lang == KOR_FONT_E)str = (uint8_t *)h_sel_recovery_79x13;
      else if(lang == ENG_FONT_E)str = (uint8_t *)e_sel_recovery_79x13;
      else if(lang == JAP_FONT_E)str = (uint8_t *)h_sel_recovery_79x13; // TBD....
    }
    else
    {
      if(lang == KOR_FONT_E)str = (uint8_t *)h_sel_create_79x13;
      else if(lang == ENG_FONT_E)str = (uint8_t *)e_sel_create_79x13;
      else if(lang == JAP_FONT_E)str = (uint8_t *)h_sel_create_79x13; // TBD....
    }
    screen_disp_bitmap(24, 10, 79, 13, str);
	
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
#ifdef D_ADD_PASSPHRASE_IN_SEED
    *next_state = PPHRASE_STATE;
    pphrase_set_state(PPHRASE_INIT_S);
#else
    if(sel_cfg_mode) // recovery
    {
      *next_state = SEED_RECOVERY_STATE;
      seed_recovery_set_state(RECOVERY_INIT_S);
    }
    else // new create
    {
      *next_state = SEED_GEN_STATE;
      seed_gen_set_state(SEED_GEN_INIT_S);
    }
#endif/* D_ADD_PASSPHRASE_IN_SEED */
    *evt_mask &= ~BUTTON_2_EVENT;

    devcfg_s = DEVCFG_NONE_S;
  }
}

void devcfg_state(uint32_t *evt_mask, uint8_t *lang, session_state_type *next_state)
{
  
  switch(devcfg_s)
  {
    case DEVCFG_NONE_S:
      
      break;
	  
    case DEVCFG_INIT_S:
      devcfg_init_disp(*lang);
      devcfg_s = DEVCFG_INIT_DISP_S;
      break;
	  
    case DEVCFG_INIT_DISP_S:
      devcfg_init_disp_event(evt_mask);
      break;
	  
    case DEVCFG_START_NOTI_INIT_S:
      devcfg_start_noti_init(*lang);
      devcfg_s = DEVCFG_START_NOTI_S;
      break;
	  
    case DEVCFG_START_NOTI_S:
      devcfg_start_noti_event(evt_mask);
      break;

    case DEVCFG_SEL_CFG_INIT_S:
      devcfg_select_cfg_init(*lang);
      devcfg_s = DEVCFG_SEL_CFG_S;
      break;
	  
    case DEVCFG_SEL_CFG_S:
      devcfg_select_cfg_event(evt_mask, *lang, next_state);
      break;
	  
    default:
      break;
  
  }
}



