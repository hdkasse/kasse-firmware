
/**
  ******************************************************************************
  * @file    pphrase.c
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

#include "res_img.h"
#include "screen_disp.h"
#include "recovery.h"
#include "seed_gen.h"
#include "pphrase.h"

extern device_feature_type dev_feature;

typedef struct
{
  uint8_t    word[10];
  uint8_t    curr_pos;
}input_pphrase_type;

typedef enum {
  _0_5_STAGE  = 0,
  _6_9_STAGE  = 1,
  _A_F_STAGE  = 2,
  _G_L_STAGE  = 3,
  _M_R_STAGE  = 4,
  _S_X_STAGE  = 5,
  _Y_Z_STAGE  = 6,
  _SP_1_STAGE = 7,
  _SP_2_STAGE = 8,
  STAGE_MAX
} pphrase_stage_type;

#if defined ( __CC_ARM )
char str_0_5[]  = "[0 1 2 3 4 5]";
char str_6_9[]  = "[6 7 8 9 < >]";
char str_a_f[]  = "[a b c d e f]";
char str_g_l[]  = "[g h i j k l]";
char str_m_r[]  = "[m n o p q r]";
char str_s_x[]  = "[s t u v w x]";
char str_y_z[]  = "[y z ~ ! @ #]";
char str_sp_1[] = "[$ % ^ & * (]";
char str_sp_2[] = "[) ? + - = ;]";
#else
const char str_0_5[]  = "[0 1 2 3 4 5]";
const char str_6_9[]  = "[6 7 8 9 < >]";
const char str_a_f[]  = "[a b c d e f]";
const char str_g_l[]  = "[g h i j k l]";
const char str_m_r[]  = "[m n o p q r]";
const char str_s_x[]  = "[s t u v w x]";
const char str_y_z[]  = "[y z ~ ! @ #]";
const char str_sp_1[] = "[$ % ^ & * (]";
const char str_sp_2[] = "[) ? + - = ;]";
#endif

const char pphrase_char_array[9][6] =
{
  {'0','1','2','3','4','5'},
  {'6','7','8','9','<','>'},
  {'a','b','c','d','e','f'},
  {'g','h','i','j','k','l'},
  {'m','n','o','p','q','r'},
  {'s','t','u','v','w','x'},
  {'y','z','~','!','@','#'},
  {'$','%','^','&','*','('},
  {')','?','+','-','=',';'}
};

static char passphrase[11] = {0,};
static char comp_pphrase[11] = {0,};

static uint8_t pphrase_len = 0;

static pphrase_state_type pp_state = PPHRASE_NONE_S;
static pphrase_stage_type pphrase_s;
static input_pphrase_type in_pphrase;
static uint8_t pphrase_offset = 0;

char *pphrase_get_passphrase(void)
{
  if(pphrase_len == 0)return "";

  return passphrase;
}

void pphrase_set_state(pphrase_state_type state)
{
  pp_state = state;
}

void pphrase_init_disp(uint8_t lang)
{
  uint8_t *str = NULL;
  

  screen_disp_clear();

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(1, 11, "Do you want to", &Font_7x10);
    screen_disp_str(1, 23, "add a passphrase ?", &Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_add_passphrase_100x25;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_add_passphrase_100x25; // TBD...
    
    screen_disp_bitmap(14, 6, 100, 25, str);
  }
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);

  memset(passphrase, 0x0, sizeof(passphrase));
  pphrase_len = 0;
}


void pphrase_input_init_disp(void)
{
  screen_disp_clear();

  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_up_7x4);
  screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
  
  pphrase_s = _0_5_STAGE;
  pphrase_offset = 0;
  screen_disp_str(17, 3, str_0_5, &Font_7x10);
  memset(&in_pphrase, 0x0, sizeof(input_pphrase_type));

  screen_disp_code_init();
  screen_disp_underline(in_pphrase.curr_pos+1, 0);
  screen_disp_code_char(in_pphrase.curr_pos+1, pphrase_char_array[pphrase_s][pphrase_offset]);
  in_pphrase.word[in_pphrase.curr_pos] = pphrase_char_array[pphrase_s][pphrase_offset];
}

void pphrase_input_passphrase(uint32_t *evt_mask)
{
  uint8_t *str;
  
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    pphrase_offset++;
    if(pphrase_offset > (6-1))
    {
      pphrase_offset = 0;
    }
    screen_disp_underline(in_pphrase.curr_pos+1, 0);
    screen_disp_code_char(in_pphrase.curr_pos+1, pphrase_char_array[pphrase_s][pphrase_offset]);
    in_pphrase.word[in_pphrase.curr_pos] = pphrase_char_array[pphrase_s][pphrase_offset];
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    pphrase_offset = 0;
    pphrase_s++;
    if(pphrase_s > _SP_2_STAGE)
    {
      pphrase_s = _0_5_STAGE;
    }
	
    if(pphrase_s == _0_5_STAGE)str = str_0_5;
    else if(pphrase_s == _6_9_STAGE)str = str_6_9;
    else if(pphrase_s == _A_F_STAGE)str = str_a_f;
    else if(pphrase_s == _G_L_STAGE)str = str_g_l;
    else if(pphrase_s == _M_R_STAGE)str = str_m_r;
    else if(pphrase_s == _S_X_STAGE)str = str_s_x;
    else if(pphrase_s == _Y_Z_STAGE)str = str_y_z;
    else if(pphrase_s == _SP_1_STAGE)str = str_sp_1;
    else if(pphrase_s == _SP_2_STAGE)str = str_sp_2;
	screen_disp_str(17, 3, str, &Font_7x10);

    screen_disp_underline(in_pphrase.curr_pos+1, 0);
    screen_disp_code_char(in_pphrase.curr_pos+1, pphrase_char_array[pphrase_s][pphrase_offset]);
    in_pphrase.word[in_pphrase.curr_pos] = pphrase_char_array[pphrase_s][pphrase_offset];
	
    *evt_mask &= ~BUTTON_2_EVENT;
  
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    in_pphrase.word[in_pphrase.curr_pos] = pphrase_char_array[pphrase_s][pphrase_offset];

    if(in_pphrase.curr_pos < 9)
    {
      screen_disp_code_char(in_pphrase.curr_pos+1, '*');
	  
      in_pphrase.curr_pos++;
      pphrase_s = 0;
      pphrase_offset = 0;
      screen_disp_underline(in_pphrase.curr_pos+1, 0);
      screen_disp_code_char(in_pphrase.curr_pos+1, pphrase_char_array[pphrase_s][pphrase_offset]);
	  in_pphrase.word[in_pphrase.curr_pos] = pphrase_char_array[pphrase_s][pphrase_offset];

      screen_disp_str(17, 3, str_0_5, &Font_7x10);
    }

    *evt_mask &= ~BUTTON_1_2_EVENT;
  }
  else if((*evt_mask & BUTTON_2_LONG_EVENT) == BUTTON_2_LONG_EVENT)  // backspace
  {
    if(in_pphrase.curr_pos > 0)
    {
      in_pphrase.word[in_pphrase.curr_pos] = 0;
      screen_disp_code_char(in_pphrase.curr_pos+1, ' ');
      screen_disp_underline(in_pphrase.curr_pos+1, 1);

      screen_disp_code_char(in_pphrase.curr_pos, in_pphrase.word[in_pphrase.curr_pos - 1]);
	  in_pphrase.curr_pos--;
      pphrase_offset = 0;
    }
    *evt_mask &= ~BUTTON_2_LONG_EVENT;
  
  }
  else if((*evt_mask & BUTTON_1_2_LONG_EVENT) == BUTTON_1_2_LONG_EVENT)
  {
    pp_state = PPHRASE_FINAL_INIT_S;
    *evt_mask &= ~BUTTON_1_2_LONG_EVENT;
  }  
}

void pphrase_final_init(uint8_t lang)
{
  uint8_t *str;
  
  memset(passphrase, 0x0, sizeof(passphrase));
  memcpy(passphrase, in_pphrase.word, in_pphrase.curr_pos+1);
  pphrase_len = strlen(passphrase);
  
  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
  screen_disp_str(62-(pphrase_len/2)*7, 20, passphrase, &Font_7x10);

  if(lang == ENG_FONT_E)str = (uint8_t *)e_pphrase_noti_90x12;
  else if(lang == KOR_FONT_E)str = (uint8_t *)h_pphrase_noti_90x12;
  else if(lang == JAP_FONT_E)str = (uint8_t *)j_pphrase_noti_90x12;
  screen_disp_bitmap(18, 1, 90, 12, str);
}

void pphrase_set_seed_state(session_state_type *next_state)
{
  if(devcfg_get_cfg_mode())
  {
    *next_state = SEED_RECOVERY_STATE;
    seed_recovery_set_state(RECOVERY_INIT_S);
  }
  else
  {
    *next_state = SEED_GEN_STATE;
    seed_gen_set_state(SEED_GEN_INIT_S);
  }
}

void pphrase_state(uint32_t *evt_mask, uint8_t *lang, session_state_type *next_state)
{
  switch(pp_state)
  {
    case PPHRASE_NONE_S:
      break;
	  
    case PPHRASE_INIT_S:
      pphrase_init_disp(*lang);
      pp_state = PPHRASE_INIT_DISP_S;
      break;

    case PPHRASE_INIT_DISP_S:
	  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
		*evt_mask &= ~BUTTON_1_EVENT;
		pp_state = PPHRASE_INPUT_INIT_S;
      }      
	  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
		*evt_mask &= ~BUTTON_2_EVENT;
		pp_state = PPHRASE_NONE_S;
		pphrase_set_seed_state(next_state);
      }

	  break;
    
    case PPHRASE_INPUT_INIT_S:
      pphrase_input_init_disp();
	  pp_state = PPHRASE_INPUT_PASSPHRASE_S;
      break;
	  
    case PPHRASE_INPUT_PASSPHRASE_S:
      pphrase_input_passphrase(evt_mask);
      break;
    
	case PPHRASE_FINAL_INIT_S:
       pphrase_final_init(*lang);
	   pp_state = PPHRASE_FINAL_DISP_S;
	   break;

	case PPHRASE_FINAL_DISP_S:
	   if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
	   {
		 *evt_mask &= ~BUTTON_1_EVENT;
		 pp_state = PPHRASE_NONE_S;
		 pphrase_set_seed_state(next_state);
	   }
	   else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
	   {
		 *evt_mask &= ~BUTTON_2_EVENT;
		 pp_state = PPHRASE_INIT_S;
	   }	  
	   break;

    default:
      break;
  }
}

void pphrase_confirm_init_disp(uint8_t lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(5, 11, "Enter again", &Font_7x10);
    screen_disp_str(5, 23, "your passphrase", &Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_same_passphrase_100x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_same_passphrase_100x28; // TBD....
    
    screen_disp_bitmap(7, 4, 100, 28, str);
  }
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void pphrase_confirm_final_init(uint8_t lang)
{
  int len;
  uint8_t *str;
  
  memset(comp_pphrase, 0x0, sizeof(comp_pphrase));
  memcpy(comp_pphrase, in_pphrase.word, in_pphrase.curr_pos+1);
  len = strlen(comp_pphrase);
  
  screen_disp_clear();
  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
  screen_disp_str(62-(len/2)*7, 20, comp_pphrase, &Font_7x10);

  
  if(lang == ENG_FONT_E)str = (uint8_t *)e_pphrase_noti_90x12;
  else if(lang == KOR_FONT_E)str = (uint8_t *)h_pphrase_noti_90x12;
  else if(lang == JAP_FONT_E)str = (uint8_t *)j_pphrase_noti_90x12;
  screen_disp_bitmap(18, 5, 90, 12, str);
}


void pphrase_confirm_state(uint32_t *evt_mask, uint8_t *lang)
{
  switch(pp_state)
  {
    case PPHRASE_NONE_S:
      break;
	  
    case PPHRASE_INIT_S:
      pphrase_confirm_init_disp(*lang);
      pp_state = PPHRASE_INIT_DISP_S;
      break;

    case PPHRASE_INIT_DISP_S:
	  if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
		*evt_mask &= ~BUTTON_2_EVENT;
		pp_state = PPHRASE_INPUT_INIT_S;
      }
	  break;
    
    case PPHRASE_INPUT_INIT_S:
      pphrase_input_init_disp();
	  pp_state = PPHRASE_INPUT_PASSPHRASE_S;
      break;
	  
    case PPHRASE_INPUT_PASSPHRASE_S:
      pphrase_input_passphrase(evt_mask);
      break;
    
	case PPHRASE_FINAL_INIT_S:
	   pphrase_confirm_final_init(*lang);
	   pp_state = PPHRASE_FINAL_DISP_S;
	   break;

	case PPHRASE_FINAL_DISP_S:
	   if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
	   {
		 *evt_mask &= ~BUTTON_1_EVENT;
		 pp_state = PPHRASE_NONE_S;

         if(!strcmp(passphrase, comp_pphrase))
         {
           *evt_mask |= CONFIRM_PPHRASE_EVENT;
         }
         else
         {
           *evt_mask |= MISMATCH_PPHRASE_EVENT;
         }
		 
	   }
	   else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
	   {
		 *evt_mask &= ~BUTTON_2_EVENT;
		 pp_state = PPHRASE_INIT_S;
	   }	  
	   break;

    default:
      break;
  }
}

