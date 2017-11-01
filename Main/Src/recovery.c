
/**
  ******************************************************************************
  * @file    recovery.c
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

#include "screen_disp.h"
#include "res_img.h"
#include "user_type.h"
#include "recovery.h"
#include "bip39.h"
#include "flash_dload.h"
#include "apdu_i2c.h"
#include "pphrase.h"

#define ALPHABET_SET 5
#define ALPHABET_POS 6

typedef struct
{
  uint8_t *word[30];
  uint8_t cnt;
}srch_list_type;

extern device_feature_type dev_feature;

static recovery_state_type r_state = RECOVERY_NONE_S;
static uint16_t recovery_word_cnt = 0;
static uint8_t input_word_order = 0;
static uint8_t partial_word[10];
static uint8_t recovery_seed[512/8];
static char *sel_word[24];
static uint8_t sel_index = 0;
static char recovery_mnemonic[24*10];

static srch_list_type srchlist;
static uint8_t srch_pos = 0;

typedef struct
{
  uint8_t    word[10];
  uint8_t    curr_pos;
}input_word_type;

typedef enum {
  A_F_STAGE = 0,
  G_L_STAGE = 1,
  M_R_STAGE = 2,
  S_X_STAGE = 3,
  Y_Z_STAGE = 4,
  STAGE_MAX
} alphabet_stage_type;

const char alphabet_char_array[ALPHABET_SET][ALPHABET_POS] =
{
  {'A','B','C','D','E','F'},
  {'G','H','I','J','K','L'},
  {'M','N','O','P','Q','R'},
  {'S','T','U','V','W','X'},
  {'Y','Z','Y','Z','Y','Z'}
};

#if defined ( __CC_ARM )
char str_AF[]  = "[A B C D E F]";
char str_GL[]  = "[G H I J K L]";
char str_MR[]  = "[M N O P Q R]";
char str_SX[]  = "[S T U V W X]";
char str_YZ[]  = "[   Y   Z   ]";
#else
const char str_AF[]  = "[A B C D E F]";
const char str_GL[]  = "[G H I J K L]";
const char str_MR[]  = "[M N O P Q R]";
const char str_SX[]  = "[S T U V W X]";
const char str_YZ[]  = "[   Y   Z   ]";
#endif
static alphabet_stage_type alphabet_s;
static input_word_type in_word;
static uint8_t alphabet_offset = 0;

void seed_recovery_set_state(recovery_state_type state)
{
  r_state = state;
}

void seed_recovery_init_disp(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(15,8,"Wallet Recovery",&Font_7x10);
    screen_disp_str(15,20,"Process",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_recovery_init_95x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_recovery_init_95x28; // TBD....
    
    screen_disp_bitmap(15, 4, 95, 28, str);
  }
  
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  
}

void seed_recovery_set_word_disp(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  if(lang == ENG_FONT_E)
  {
    screen_disp_str(4,8,"Set the number of",&Font_7x10);
    screen_disp_str(4,20,"words to recover",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_num_words_95x32;
    else if(lang == JAP_FONT_E)str = (uint8_t *)j_num_words_95x32;
    
    screen_disp_bitmap(15, 0, 95, 32, str);
  }

}

void seed_recovery_word_count_disp(void)
{
  screen_disp_clear();

  input_word_order = 0;
  recovery_word_cnt = WORDS_S24;
  screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_24_word_30x15);
  
  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_down_7x4);
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(60, 25, 7, 6, (uint8_t*)next_menu_exist_7x6);
}

void seed_recovery_word_count_sel(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(recovery_word_cnt == WORDS_S24)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_21_word_30x15);
      recovery_word_cnt = WORDS_S21;
    }
    else if(recovery_word_cnt == WORDS_S21)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_18_word_30x15);
      recovery_word_cnt = WORDS_S18;
    }
    else if(recovery_word_cnt == WORDS_S18)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_15_word_30x15);
      recovery_word_cnt = WORDS_S15;
    }
    else if(recovery_word_cnt == WORDS_S15)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_12_word_30x15);
      recovery_word_cnt = WORDS_S12;
    }
    else if(recovery_word_cnt == WORDS_S12)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_9_word_30x15);
      recovery_word_cnt = WORDS_S09;
    }
    else if(recovery_word_cnt == WORDS_S09)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_6_word_30x15);
      recovery_word_cnt = WORDS_S06;
    }
    else if(recovery_word_cnt == WORDS_S06)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_3_word_30x15);
      recovery_word_cnt = WORDS_S03;
    }
    else if(recovery_word_cnt == WORDS_S03)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_24_word_30x15);
      recovery_word_cnt = WORDS_S24;
    }
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    r_state = RECOVERY_WORD_SEL_INTRO_INIT_S;
  }
}

void seed_recovery_word_sel_intro(uint8_t lang)
{
  uint8_t *str = NULL;
  
  if(lang == KOR_FONT_E)str = (uint8_t *)h_input_word_50x15;
  else if(lang == ENG_FONT_E)str = (uint8_t *)e_input_word_50x15;
  else if(lang == JAP_FONT_E)str = (uint8_t *)j_input_word_50x15;

  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_str(50, 4, "#", &Font_7x10);
  screen_disp_num(65, 4, input_word_order+1, &Font_7x10);
  screen_disp_bitmap(39, 17, 50, 15, str);
}

void seed_recovery_input_init(void)
{
  screen_disp_clear();

  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_up_7x4);
  screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);

  memset(partial_word, 0x0, sizeof(partial_word));
  
  alphabet_s = A_F_STAGE;
  alphabet_offset = 0;
  screen_disp_str(17, 2, str_AF, &Font_7x10);
  memset(&in_word, 0x0, sizeof(in_word));
  in_word.curr_pos = 0;
  screen_disp_code_init();
  screen_disp_underline(in_word.curr_pos+1, 0);
  screen_disp_code_char(in_word.curr_pos+1, alphabet_char_array[alphabet_s][alphabet_offset]);
}

void seed_recovery_input_word(uint32_t *evt_mask)
{
  uint8_t *str;
  int i;
  
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    alphabet_offset++;
    if(alphabet_offset > (ALPHABET_POS-1))
    {
      alphabet_offset = 0;
    }
    screen_disp_underline(in_word.curr_pos+1, 0);
    screen_disp_code_char(in_word.curr_pos+1, alphabet_char_array[alphabet_s][alphabet_offset]);
    in_word.word[in_word.curr_pos] = alphabet_char_array[alphabet_s][alphabet_offset];
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    alphabet_offset = 0;
    alphabet_s++;
    if(alphabet_s > Y_Z_STAGE)
    {
      alphabet_s = A_F_STAGE;
    }
    if(alphabet_s == A_F_STAGE)str = str_AF;
    else if(alphabet_s == G_L_STAGE)str = str_GL;
    else if(alphabet_s == M_R_STAGE)str = str_MR;
    else if(alphabet_s == S_X_STAGE)str = str_SX;
    else if(alphabet_s == Y_Z_STAGE)str = str_YZ;
    else str = str_AF;
	
    screen_disp_str(17, 2, str, &Font_7x10);

    screen_disp_underline(in_word.curr_pos+1, 0);
    screen_disp_code_char(in_word.curr_pos+1, alphabet_char_array[alphabet_s][alphabet_offset]);
    in_word.word[in_word.curr_pos] = alphabet_char_array[alphabet_s][alphabet_offset];
	
    *evt_mask &= ~BUTTON_2_EVENT;
  
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    in_word.word[in_word.curr_pos] = alphabet_char_array[alphabet_s][alphabet_offset];
    if(in_word.curr_pos == 2)
    {
      for(i=0;i<3;i++)
      {
        partial_word[i] = tolower(in_word.word[i]);
      }
      r_state = RECOVERY_FULL_WORD_SEL_INIT_S;
    }
    else
    {
      in_word.curr_pos++;
      alphabet_s = 0;
      alphabet_offset = 0;
      str = str_AF;
      screen_disp_underline(in_word.curr_pos+1, 0);
      screen_disp_code_char(in_word.curr_pos+1, alphabet_char_array[alphabet_s][alphabet_offset]);

      screen_disp_str(17, 2, str, &Font_7x10);
    }

    *evt_mask &= ~BUTTON_1_2_EVENT;
  }
  else if((*evt_mask & BUTTON_2_LONG_EVENT) == BUTTON_2_LONG_EVENT)  // backspace
  {
    if(in_word.curr_pos > 0)
    {
      in_word.word[in_word.curr_pos] = 0;
      screen_disp_code_char(in_word.curr_pos+1, ' ');
      screen_disp_underline(in_word.curr_pos+1, 1);
      in_word.curr_pos--;
      alphabet_offset = 0;
    }
    *evt_mask &= ~BUTTON_2_LONG_EVENT;
  
  }
  
}

void seed_recovery_search_word(void)
{
  int i;
  char **list = (char **)mnemonic_wordlist();
 
  srchlist.cnt = 0;
  
  for(i=0;list[i] != 0;i++)
  {
    if(!memcmp(partial_word, list[i], 3))
    {
      srchlist.word[srchlist.cnt++] = list[i];
    }
  }
}

void seed_recovery_word_sel_init(void)
{
  int len;

  screen_disp_clear();
  
  if(srchlist.cnt == 0)
  {
    r_state = RECOVERY_NO_MATCH_WORD_INIT_S;
  }
  else
  {  
    screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
    srch_pos = 0;

    screen_disp_str(45, 2, "#", &Font_7x10);
    screen_disp_num(58, 2, srch_pos+1, &Font_7x10);
    screen_disp_str(66, 2, "/", &Font_7x10);
    screen_disp_num(73, 2, srchlist.cnt, &Font_7x10);
		
    len = strlen(srchlist.word[srch_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
    screen_disp_str2(60-len*5,15, srchlist.word[srch_pos], &Font_11x18);
#else
    screen_disp_str(60-len*5,15, srchlist.word[srch_pos], &Font_11x18);
#endif
#else
    screen_disp_str(60-len*5,20, srchlist.word[srch_pos], &Font_7x10);
#endif
	
    r_state = RECOVERY_FULL_WORD_SEL_S;
  }
}

void seed_recovery_word_sel(uint32_t *evt_mask, uint8_t lang)
{
  uint8_t *str = NULL;
  int len;
  int i;
  
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(srch_pos > 0)
    {
      screen_disp_clear();
      srch_pos--;
      len = strlen(srchlist.word[srch_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
      screen_disp_str2(60-len*5,15, srchlist.word[srch_pos], &Font_11x18);
#else
      screen_disp_str(60-len*5,15, srchlist.word[srch_pos], &Font_11x18);
#endif
#else
      screen_disp_str(60-len*5,20, srchlist.word[srch_pos], &Font_7x10);
#endif
      if(srch_pos > 0)
      {
        screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
      }
      if(srch_pos < (srchlist.cnt))
      {
        screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
      }
	  
      screen_disp_str(45, 2, "#", &Font_7x10);
      screen_disp_num(58, 2, srch_pos+1, &Font_7x10);
      screen_disp_str(66, 2, "/", &Font_7x10);
      screen_disp_num(73, 2, srchlist.cnt, &Font_7x10);
    }
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    if(srch_pos < (srchlist.cnt-1))
    {
      screen_disp_clear();
      srch_pos++;
      len = strlen(srchlist.word[srch_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
      screen_disp_str2(60-len*5,15, srchlist.word[srch_pos], &Font_11x18);
#else
      screen_disp_str(60-len*5,15, srchlist.word[srch_pos], &Font_11x18);
#endif
#else
      screen_disp_str(60-len*5,20, srchlist.word[srch_pos], &Font_7x10);
#endif

      if(srch_pos > 0)
      {
        screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
      }
      if(srch_pos < (srchlist.cnt))
      {
        screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
      }
      screen_disp_str(45, 2, "#", &Font_7x10);
      screen_disp_num(58, 2, srch_pos+1, &Font_7x10);
      screen_disp_str(66, 2, "/", &Font_7x10);
      screen_disp_num(73, 2, srchlist.cnt, &Font_7x10);
    }
    else if(srch_pos == (srchlist.cnt-1))
    {
      srch_pos++;
      screen_disp_clear();
      screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
      screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
	  
      if(lang == KOR_FONT_E)str = (uint8_t *)h_cancel_word_79x16;
      else if(lang == ENG_FONT_E)str = (uint8_t *)e_cancel_word_79x16;
      else if(lang == JAP_FONT_E)str = (uint8_t *)j_cancel_word_79x16;
	  
      screen_disp_bitmap(24, 8, 79, 16, str);
      
    }
    else if(srchlist.word[srch_pos] == 0 && srch_pos == srchlist.cnt)
    {
      r_state = RECOVERY_WORD_SEL_INTRO_INIT_S;
    }
	
    *evt_mask &= ~BUTTON_2_EVENT;
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    
    //if(srchlist.word[srch_pos] == 0)
    //{
    //  r_state = RECOVERY_WORD_SEL_INTRO_INIT_S;
    //}
    //else
    {    
      sel_word[sel_index++] = (char*)srchlist.word[srch_pos];
      if(input_word_order < (recovery_word_cnt*3/32 -1))
      {
        input_word_order++;
        r_state = RECOVERY_WORD_SEL_INTRO_INIT_S;
      }
      else
      {
        char *p = recovery_mnemonic;
        for(i=0;i<sel_index;i++)
        {
          strcpy(p, sel_word[i]);
          p += strlen(sel_word[i]);
          *p = (i < sel_index - 1) ? ' ' : 0;
          p++;
        }
        //r_state = RECOVERY_SEED_GENERATE_S;
        r_state = RECOVERY_INPUT_COMPLETE_INIT_S;
      }
	  	
    }
    *evt_mask &= ~BUTTON_1_2_EVENT;
  }
}

void seed_recovery_create_seed(uint8_t lang)
{
  uint8_t *str = NULL;
  char *passphrase = NULL;

  screen_disp_clear();
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(10,8,"Wallet Recovery",&Font_7x10);
    screen_disp_str(10,20,"in progress...",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_recovery_in_prog_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_recovery_in_prog_90x28; // TBD..
    
    screen_disp_bitmap(15, 4, 90, 28, str);
  }
#if defined ( __CC_ARM )
  passphrase = (char *)pphrase_get_passphrase();
#else    
  passphrase = pphrase_get_passphrase();
#endif
  mnemonic_to_seed(recovery_mnemonic, passphrase, recovery_seed, NULL);
}

void seed_recovery_create_completed(uint8_t lang, uint8_t status)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  if(status)
  {
    if(lang == ENG_FONT_E)
    {
      screen_disp_str(10,8,"Wallte has been",&Font_7x10);
      screen_disp_str(10,20,"recovered",&Font_7x10);      
    }
    else
    {
      if(lang == KOR_FONT_E)str = (uint8_t *)h_recovery_complete_90x28;
      else if(lang == JAP_FONT_E)str = (uint8_t *)h_recovery_complete_90x28; // TBD...
      screen_disp_bitmap(15, 4, 90, 28, str);
    }
  }
  else
  {
    //if(lang == ENG_FONT_E)
    {
      screen_disp_str(2,8,"Recovery failed",&Font_7x10);
      screen_disp_str(2,20,"Retry Again",&Font_7x10);
    }
    //else
    //{
    //  if(lang == KOR_FONT_E)str = (uint8_t *)h_seed_save_error_90x28;
    //  else if(lang == JAP_FONT_E)str = (uint8_t *)h_seed_save_error_90x28;
    //  screen_disp_bitmap(18, 2, 90, 28, str);
    //}
  }

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void seed_recovery_input_complete(uint8_t lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();
  
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(15,8,"Complete",&Font_7x10);
    screen_disp_str(15,20,"Start Recovery",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_recovery_start_noti_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_recovery_start_noti_90x28; // TBD..
    
    screen_disp_bitmap(18, 2, 90, 28, str);
  }
  
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void seed_recovery_no_match_word(uint8_t lang)
{
  uint8_t *str = NULL;
  
  screen_disp_clear();
  
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(5,8,"No matches found",&Font_7x10);
    screen_disp_str(5,20,"Retry again",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_fail_srch_word_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)j_fail_srch_word_90x28;
    
    screen_disp_bitmap(18, 2, 90, 28, str);
  }
  
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  
}

void seed_recovery_state(uint32_t *evt_mask, uint8_t *lang)
{
  uint8_t seed_result;

  switch(r_state)
  {
    case RECOVERY_NONE_S:
      break;
	  
    case RECOVERY_INIT_S:
      memset(recovery_mnemonic, 0x0, sizeof(recovery_mnemonic));
	  sel_index = 0;
      seed_recovery_init_disp(*lang);
      r_state = RECOVERY_INIT_DISP_S;
      break;

    case RECOVERY_INIT_DISP_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        r_state = RECOVERY_WORD_CNT_NOTI_INIT_S;
      }
      break;

    case RECOVERY_WORD_CNT_NOTI_INIT_S:
      seed_recovery_set_word_disp(*lang);
      r_state = RECOVERY_WORD_CNT_NOTI_S;
      break;
    case RECOVERY_WORD_CNT_NOTI_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        r_state = RECOVERY_WORD_CNT_SEL_INIT_S;
      }
      break;
	  
    case RECOVERY_WORD_CNT_SEL_INIT_S:
      seed_recovery_word_count_disp();
      r_state = RECOVERY_WORD_CNT_SEL_S;
      break;
    case RECOVERY_WORD_CNT_SEL_S:
      seed_recovery_word_count_sel(evt_mask);
      break;
    
    case RECOVERY_WORD_SEL_INTRO_INIT_S:
      seed_recovery_word_sel_intro(*lang);
      r_state = RECOVERY_WORD_SEL_INTRO_S;
      break;
    case RECOVERY_WORD_SEL_INTRO_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        r_state = RECOVERY_WORD_INPUT_INIT_S;
      }
      break;
    
    case RECOVERY_WORD_INPUT_INIT_S:
      seed_recovery_input_init();
      r_state = RECOVERY_WORD_INPUT_S;
      break;
    case RECOVERY_WORD_INPUT_S:
      seed_recovery_input_word(evt_mask);
      break;
    
    case RECOVERY_FULL_WORD_SEL_INIT_S:
      seed_recovery_search_word();
      seed_recovery_word_sel_init();
      break;
    case RECOVERY_FULL_WORD_SEL_S:
      seed_recovery_word_sel(evt_mask, *lang);
      break;

    case RECOVERY_NO_MATCH_WORD_INIT_S:
      seed_recovery_no_match_word(*lang);
      r_state = RECOVERY_NO_MATCH_WORD_S;
      break;
    case RECOVERY_NO_MATCH_WORD_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        r_state = RECOVERY_WORD_SEL_INTRO_INIT_S;
      }
      break;

    case RECOVERY_INPUT_COMPLETE_INIT_S:
      seed_recovery_input_complete(*lang);
      r_state = RECOVERY_INPUT_COMPLETE_S;
      break;

    case RECOVERY_INPUT_COMPLETE_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        r_state = RECOVERY_SEED_GENERATE_S;
      }
      break;
      
    case RECOVERY_SEED_GENERATE_S:
      seed_recovery_create_seed(*lang);
      r_state = RECOVERY_SEND_APDU_S;
      break;

    case RECOVERY_SEND_APDU_S:
      seed_result = APDU_set_init_seed(recovery_seed, 64);
      seed_recovery_create_completed(*lang, seed_result);

      flash_fd_has_seed(seed_result ? 1:0);
      dev_feature.has_seed = seed_result ? 1:0;
      r_state = RECOVERY_COMPLETE_S;
      break;
	  
    case RECOVERY_COMPLETE_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        r_state = RECOVERY_NONE_S;
        *evt_mask |= SEED_RECOVERY_EVENT;
        *evt_mask &= ~BUTTON_2_EVENT;
      }
      break;

    default:
      break;
  }
}


