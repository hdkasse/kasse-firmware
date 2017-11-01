/**
  ******************************************************************************
  * @file    seed_gen.c
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

#include "seed_gen.h"
#include "screen_disp.h"
#include "res_img.h"
#include "user_type.h"
#include "flash_dload.h"

#include "bip39.h"
#include "apdu_i2c.h"
#include "pphrase.h"
#include "rand.h"

extern words_list_type rand_words;
extern device_feature_type dev_feature;

static words_list_type confirm_words;
static uint8_t rand_order[3] = {1, 2, 3};
static uint8_t ro_index;
static uint8_t rng_buff[24];
static uint8_t verified_pphrase;

static seed_gen_state_type seed_gen_s = SEED_GEN_NONE_S;
static uint8_t word_pos = 0;
static uint16_t words_count = 0;
char *mnemonic = NULL;
static uint8_t new_seed[512/8];

#define MAX_RETRY_COUNT 3
static uint8_t repeat_fail = 0;

void seed_gen_set_state(seed_gen_state_type state)
{
  seed_gen_s = state;
}

void seed_gen_init_disp(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(22,8,"Record words", &Font_7x10);
    screen_disp_str(22,20,"for recovery", &Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_recovery_phrase_95x32;
    else if(lang == JAP_FONT_E)str = (uint8_t *)j_recovery_phrase_95x32;
    
    screen_disp_bitmap(15, 0, 95, 32, str);
  }

  repeat_fail = 0;
}

void seed_gen_set_word_disp(uint8_t lang)
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

void seed_gen_words_disp(void)
{
  screen_disp_clear();
  screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
}

void seed_gen_words_disp_init(void)
{
  uint8_t len = 0;
  
  word_pos = 0;
  seed_gen_words_disp();
  screen_disp_str(33,2, "#", &Font_7x10);
  screen_disp_num(51,2, word_pos+1, &Font_7x10);
  screen_disp_str(65,2, "/", &Font_7x10);
  screen_disp_num(74,2, words_count*3/32, &Font_7x10);
  len = strlen(rand_words.words[word_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
  screen_disp_str2(60-len*5,15, rand_words.words[word_pos], &Font_11x18);
#else
  screen_disp_str(60-len*5,15, rand_words.words[word_pos], &Font_11x18);
#endif
#else
  screen_disp_str(60-len*5,20, rand_words.words[word_pos], &Font_7x10);
#endif
}

void seed_gen_word_disp(uint32_t *evt_mask, uint8_t lang)
{
  uint8_t len = 0;
  uint8_t *str = NULL;
  
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(word_pos > 0)
    {
      if(word_pos == (words_count*3/32))
      {
        seed_gen_words_disp();
        screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
        screen_disp_str(33,2, "#", &Font_7x10);
        screen_disp_str(65,2, "/", &Font_7x10);
        screen_disp_num(74,2, words_count*3/32, &Font_7x10);
      }
      word_pos--;
      if(word_pos == 0)
      {
        seed_gen_words_disp_init();
      }
      else if(word_pos == (words_count*3/32)+1)
      {
        screen_disp_clear();
        screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
        screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
        if(lang == ENG_FONT_E)
        {
          screen_disp_str(4,8,"Be sure to record", &Font_7x10);
          screen_disp_str(4,20,"Please keep it", &Font_7x10);
        }
        else
        {
          if(lang == KOR_FONT_E)str = (uint8_t *)h_word_list_2_100x28;
          else if(lang == JAP_FONT_E)str = (uint8_t *)h_word_list_2_100x28; // TBD..
          screen_disp_bitmap(14, 2, 100, 28, str);
        }
      }
      else if(word_pos == (words_count*3/32))
      {
        screen_disp_clear();
        screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
        screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
        if(lang == ENG_FONT_E)
        {
          screen_disp_str(4,8,"If lost word list", &Font_7x10);
          screen_disp_str(19,20,"Unrecoverable", &Font_7x10);
        }
        else
        {
          if(lang == KOR_FONT_E)str = (uint8_t *)h_word_list_1_100x28;
          else if(lang == JAP_FONT_E)str = (uint8_t *)h_word_list_1_100x28; // TBD..
          screen_disp_bitmap(14, 2, 100, 28, str);
        }	  		
      }
      else
      {
        screen_disp_str(51,2, "  ", &Font_7x10);
        screen_disp_num(51,2, word_pos+1, &Font_7x10);
        len = strlen(rand_words.words[word_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
        screen_disp_str2(0,15, "            ", &Font_11x18);
        screen_disp_str2(60-len*5,15, rand_words.words[word_pos], &Font_11x18);
#else
        screen_disp_str(0,15, "            ", &Font_11x18);
        screen_disp_str(60-len*5,15, rand_words.words[word_pos], &Font_11x18);
#endif
#else
        screen_disp_str(0,20, "                 ", &Font_7x10);
        screen_disp_str(60-len*5,20, rand_words.words[word_pos], &Font_7x10);
#endif
      }
    }
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    if(word_pos < (words_count*3/32 - 1))
    {
      word_pos++;
      screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
      screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
      screen_disp_str(51,2, "  ", &Font_7x10);
      screen_disp_num(51,2, word_pos+1, &Font_7x10);
      len = strlen(rand_words.words[word_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
      screen_disp_str2(0,15, "            ", &Font_11x18);
      screen_disp_str2(60-len*5,15, rand_words.words[word_pos], &Font_11x18);
#else
      screen_disp_str(0,15, "            ", &Font_11x18);
      screen_disp_str(60-len*5,15, rand_words.words[word_pos], &Font_11x18);
#endif
#else
      screen_disp_str(0,20, "                 ", &Font_7x10);
      screen_disp_str(60-len*5,20, rand_words.words[word_pos], &Font_7x10);
#endif	  
    }
    else if(word_pos == (words_count*3/32 - 1))
    {
	  
      word_pos++;
      screen_disp_clear();
      screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
      screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
      if(lang == ENG_FONT_E)
      {
        screen_disp_str(4,8,"If lost word list", &Font_7x10);
        screen_disp_str(19,20,"Unrecoverable", &Font_7x10);
      }
      else
      {
        if(lang == KOR_FONT_E)str = (uint8_t *)h_word_list_1_100x28;
        else if(lang == JAP_FONT_E)str = (uint8_t *)h_word_list_1_100x28; // TBD..
        screen_disp_bitmap(14, 2, 100, 28, str);
      }	  
    }
    else if(word_pos == (words_count*3/32))
    {
      word_pos++;
      screen_disp_clear();
      screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);
      screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);

      if(lang == ENG_FONT_E)
      {
        screen_disp_str(4,8,"Be sure to record", &Font_7x10);
        screen_disp_str(4,20,"Please keep it", &Font_7x10);
      }
      else
      {
        if(lang == KOR_FONT_E)str = (uint8_t *)h_word_list_2_100x28;
        else if(lang == JAP_FONT_E)str = (uint8_t *)h_word_list_2_100x28; // TBD..
        screen_disp_bitmap(14, 2, 100, 28, str);
      }	  
    }
    else if(word_pos == (words_count*3/32 + 1))
    {
	  
      word_pos++;
      screen_disp_clear();
      screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);

      if(lang == ENG_FONT_E)
      {
        screen_disp_str(8, 8, "Press 2 buttons", &Font_7x10);
        screen_disp_str(8, 20, "for Verification", &Font_7x10);
      }
      else
      {
        if(lang == KOR_FONT_E)str = (uint8_t *)h_seed_next_step_90x28;
        else if(lang == JAP_FONT_E)str = (uint8_t *)j_seed_next_step_90x28;
        screen_disp_bitmap(15, 4, 90, 28, str);
      }
    }
    *evt_mask &= ~BUTTON_2_EVENT;
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    if(word_pos == (words_count*3/32 + 2))
    {
      //seed_gen_s = SEED_GEN_SEED_GENERATE_S;
      seed_gen_s = SEED_GEN_FINAL_CHECK_INIT_S;
    }
    *evt_mask &= ~BUTTON_1_2_EVENT;
  }
}


void seed_gen_word_count_disp(void)
{
  screen_disp_clear();

  words_count = WORDS_S24;
  screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_24_word_30x15);
  
  screen_disp_bitmap(0, 0, 7, 4, (uint8_t*)button_down_7x4);
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(60, 25, 7, 6, (uint8_t*)next_menu_exist_7x6);
}

void seed_gen_word_count_sel(uint32_t *evt_mask)
{
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(words_count == WORDS_S24)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_21_word_30x15);
      words_count = WORDS_S21;
    }
    else if(words_count == WORDS_S21)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_18_word_30x15);
      words_count = WORDS_S18;
    }
    else if(words_count == WORDS_S18)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_15_word_30x15);
      words_count = WORDS_S15;
    }
    else if(words_count == WORDS_S15)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_12_word_30x15);
      words_count = WORDS_S12;
    }
    else if(words_count == WORDS_S12)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_9_word_30x15);
      words_count = WORDS_S09;
    }
    else if(words_count == WORDS_S09)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_6_word_30x15);
      words_count = WORDS_S06;
    }
    else if(words_count == WORDS_S06)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_3_word_30x15);
      words_count = WORDS_S03;
    }
    else if(words_count == WORDS_S03)
    {
      screen_disp_bitmap(45, 10, 30, 15, (uint8_t*)_24_word_30x15);
      words_count = WORDS_S24;
    }
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    *evt_mask &= ~BUTTON_2_EVENT;
    seed_gen_s = SEED_GEN_WORDS_INIT_DISP_S;
  }
}

void seed_gen_create_seed(uint8_t lang)
{
  uint8_t *str = NULL;
  char *passphrase = NULL;

  screen_disp_clear();
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(17,8,"Creating",&Font_7x10);
    screen_disp_str(17,20,"a new wallet...",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_seed_in_prog_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)j_seed_in_prog_90x28;
    
    screen_disp_bitmap(15, 4, 90, 28, str);
  }

#if defined ( __CC_ARM )
  passphrase = (char *)pphrase_get_passphrase();
#else    
  passphrase = pphrase_get_passphrase();
#endif
  mnemonic_to_seed(mnemonic, passphrase, new_seed, NULL);
}

void seed_gen_create_completed(uint8_t lang, uint8_t status)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  if(status)
  {
    if(lang == ENG_FONT_E)
    {
      screen_disp_str(10,8,"Wallte has been",&Font_7x10);
      screen_disp_str(10,20,"created",&Font_7x10);
    }
    else
    {
      if(lang == KOR_FONT_E)str = (uint8_t *)h_seed_complete_90x28;
      else if(lang == JAP_FONT_E)str = (uint8_t *)j_seed_complete_90x28;
      screen_disp_bitmap(15, 4, 90, 28, str);
    }  
  }
  else
  {
    //if(lang == ENG_FONT_E)
    //{
      screen_disp_str(15,8,"Failed",&Font_7x10);
      screen_disp_str(15,20,"Retry Again",&Font_7x10);
    //}
    //else
    //{
    //  if(lang == KOR_FONT_E)str = (uint8_t *)h_seed_save_error_90x28;
    //  else if(lang == JAP_FONT_E)str = (uint8_t *)h_seed_save_error_90x28;
    //  screen_disp_bitmap(18, 2, 90, 28, str);
    //}
  }

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void seed_gen_final_check_init(void)
{
  char *passphrase = NULL;
  int len = 0;

  verified_pphrase = 0;
  passphrase = (char *)pphrase_get_passphrase();
  len = strlen(passphrase);
  
  seed_gen_s = (len > 0) ? SEED_GEN_FINAL_CHECK_PPHRASE_INIT_S : SEED_GEN_FINAL_CHECK_WORD_INTRO_INIT_S;
}

void seed_gen_final_check_fail_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(10,8,"Mismatched data",&Font_7x10);
    screen_disp_str(10,20,"Retry again",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_mismatch_data_95x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_mismatch_data_95x28; // TBD...
    
    screen_disp_bitmap(15, 4, 95, 28, str);
  }

  screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
  screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
}

void random_order_generate(void)
{
  int i = 0;
  int idx = 0;
  uint8_t found = 0;
  uint32_t rnum;

  memset(rand_order, 0x0, sizeof(rand_order));

  if((words_count*3/32) == 3)
  {
    rand_order[0] = 1;
    rand_order[1] = 2;
    rand_order[2] = 3;
    return;
  }
  
  do{
    rnum = random_uniform(words_count*3/32);
    if(rnum == 0)continue;

    found = 0;
    for(i = 0; i < 3; i++)
    {
      if(rand_order[i] == rnum)
      {
        found = 1;
      }
    }

    if(found)continue;
    
    if(!found)
    {
      rand_order[idx] = rnum;
    }
    idx++;

    if(idx > 2)break;
    
  }while(1);
}

void random_order_words_list(void)
{
  int i;
  uint8_t wcnt;

  wcnt = words_count*3/32;
  confirm_words.words_cnt = wcnt;
  if(wcnt == 3)
  {
    strcpy(confirm_words.words[0], rand_words.words[1]);
    strcpy(confirm_words.words[1], rand_words.words[2]);
    strcpy(confirm_words.words[2], rand_words.words[0]);
  }
  else
  {
    memset(rng_buff, 0x0, sizeof(rng_buff));
    for(i = 0; i < wcnt; i++)
    {
      rng_buff[i] = i + 1;
    }
    random_permute(rng_buff, wcnt);
    
    for(i = 0; i< wcnt; i++)
    {
      strcpy(confirm_words.words[i], rand_words.words[rng_buff[i]-1]);      
    }
  }
}

void seed_gen_final_check_words_intro(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(20,8,"Verify Words",&Font_7x10);
    screen_disp_str(20,20,"you recorded",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_verify_word_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_verify_word_90x28; // TBD....
    
    screen_disp_bitmap(15, 4, 90, 28, str);
  }
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);

  random_order_generate();
  random_order_words_list();
  ro_index = 0;
}

void seed_gen_final_words_disp_init(void)
{
  uint8_t len = 0;
  
  word_pos = 0;
  seed_gen_words_disp();
  screen_disp_str(33,2, "#", &Font_7x10);
  screen_disp_num(51,2, word_pos+1, &Font_7x10);
  screen_disp_str(65,2, "/", &Font_7x10);
  screen_disp_num(74,2, words_count*3/32, &Font_7x10);
  len = strlen(confirm_words.words[word_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
  screen_disp_str2(60-len*5,15, confirm_words.words[word_pos], &Font_11x18);
#else
  screen_disp_str(60-len*5,15, confirm_words.words[word_pos], &Font_11x18);
#endif
#else
  screen_disp_str(60-len*5,20, confirm_words.words[word_pos], &Font_7x10);
#endif
}

void seed_gen_final_words_disp(uint32_t *evt_mask, uint8_t lang)
{
  uint8_t len = 0;
  
  if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
  {
    if(word_pos > 0)
    {
      word_pos--;
      if(word_pos == 0)
      {
        seed_gen_final_words_disp_init();
      }
      else
      {
        screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
        
        screen_disp_str(51,2, "  ", &Font_7x10);
        screen_disp_num(51,2, word_pos+1, &Font_7x10);
        len = strlen(confirm_words.words[word_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
        screen_disp_str2(0,15, "            ", &Font_11x18);
        screen_disp_str2(60-len*5,15, confirm_words.words[word_pos], &Font_11x18);
#else
        screen_disp_str(0,15, "            ", &Font_11x18);
        screen_disp_str(60-len*5,15, confirm_words.words[word_pos], &Font_11x18);
#endif
#else
        screen_disp_str(0,20, "                 ", &Font_7x10);
        screen_disp_str(60-len*5,20, confirm_words.words[word_pos], &Font_7x10);
#endif
      }
    }
    *evt_mask &= ~BUTTON_1_EVENT;
  }
  else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
  {
    if(word_pos < (words_count*3/32 - 1))
    {
      screen_disp_bitmap(0, 0, 4, 7, (uint8_t*)left_button_icon);

      word_pos++;
      if(word_pos < (words_count*3/32 - 1))
      {
        screen_disp_bitmap(120, 0, 4, 7, (uint8_t*)right_button_icon);
      }
      else
      {
        screen_disp_str(120, 0, " ", &Font_7x10);
      }
      
      screen_disp_str(51,2, "  ", &Font_7x10);
      screen_disp_num(51,2, word_pos+1, &Font_7x10);
      len = strlen(confirm_words.words[word_pos]);
#ifdef D_WORD_BIG_FONT
#ifdef D_CODE_OPTIMIZE
      screen_disp_str2(0,15, "            ", &Font_11x18);
      screen_disp_str2(60-len*5,15, confirm_words.words[word_pos], &Font_11x18);
#else
      screen_disp_str(0,15, "            ", &Font_11x18);
      screen_disp_str(60-len*5,15, confirm_words.words[word_pos], &Font_11x18);
#endif
#else
      screen_disp_str(0,20, "                 ", &Font_7x10);
      screen_disp_str(60-len*5,20, confirm_words.words[word_pos], &Font_7x10);
#endif	  
    }    
    *evt_mask &= ~BUTTON_2_EVENT;
  }
  else if((*evt_mask & BUTTON_1_2_EVENT) == BUTTON_1_2_EVENT)
  {
    if(strcmp(rand_words.words[rand_order[ro_index]-1], confirm_words.words[word_pos]))
    {
      repeat_fail++;

      if(repeat_fail < MAX_RETRY_COUNT)
      {
        seed_gen_s = SEED_GEN_FINAL_FAIL_INIT_S;
      }
      else
      {
        seed_gen_s = SEED_GEN_MAX_RETRY_FAIL_INIT_S;
      }
    }
    else
    {
      repeat_fail = 0;
      
      if(ro_index < 2)
      {
        ro_index++;
        seed_gen_s = SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_INIT_S;
      }
      else
      {
        seed_gen_s = SEED_GEN_FINAL_OK_INIT_S;
      }
    }
    *evt_mask &= ~BUTTON_1_2_EVENT;
  }
}

void seed_gen_final_noti_word_init(uint8_t lang)
{
  uint8_t *str = NULL;
  uint8_t *str2 = NULL;
  char str_order[15];
  char tmp[5];

  random_order_words_list();
  memset(str_order, 0x0, sizeof(str_order));
  
  screen_disp_clear();
  
  if(lang == ENG_FONT_E)
  {
    strcpy(str_order, "# ");
  
    if(rand_order[ro_index] == 1)
    {
      strcat(str_order, "1st word");
    }
    else if(rand_order[ro_index] == 2)
    {
      strcat(str_order, "2nd word");
    }
    else if(rand_order[ro_index] == 3)
    {
      strcat(str_order, "3rd word");
    }
    else
    {
      itoa(rand_order[ro_index], tmp, 10);
      strcat(str_order, tmp);
      strcat(str_order, "th word");
    }

    screen_disp_str(5,8,"Select same word",&Font_7x10);
    screen_disp_str(5,20, str_order,&Font_7x10);

  }
  else
  {
    strcpy(str_order, "# ");
    itoa(rand_order[ro_index], tmp, 10);
    strcat(str_order, tmp);
    
    screen_disp_str(5,8,str_order,&Font_7x10);
    
    if(lang == KOR_FONT_E)
    {
      // 35
      str = (uint8_t *)h_same_th_1_60x16;
      str2 = (uint8_t *)h_same_th_2_60x16;
    }
    else if(lang == JAP_FONT_E)
    {
      str = (uint8_t *)h_same_th_1_60x16;  // TBD....
      str2 = (uint8_t *)h_same_th_2_60x16;  // TBD....
    }
    screen_disp_bitmap(35, 4, 60, 16, str);
    screen_disp_bitmap(35, 18, 60, 16, str2);
  }
  
  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void seed_gen_final_check_ok_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(10,8,"Verification",&Font_7x10);
    screen_disp_str(10,20,"is complete",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_verify_complete_90x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_verify_complete_90x28; // TBD...
    
    screen_disp_bitmap(15, 4, 90, 28, str);
  }

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void seed_gen_max_retry_fail_init(uint8_t lang)
{
  uint8_t *str = NULL;

  screen_disp_clear();
  if(lang == ENG_FONT_E)
  {
    screen_disp_str(5,8,"Max Retry #3",&Font_7x10);
    screen_disp_str(5,20,"Restart creation",&Font_7x10);
  }
  else
  {
    if(lang == KOR_FONT_E)str = (uint8_t *)h_seed_gen_max_retry_100x28;
    else if(lang == JAP_FONT_E)str = (uint8_t *)h_seed_gen_max_retry_100x28; // TBD...
    
    screen_disp_bitmap(2, 4, 100, 28, str);
  }

  screen_disp_bitmap(116, 0, 9, 7, (uint8_t*)confirm_check_icon);
}

void seed_gen_state(uint32_t *evt_mask, uint8_t *lang)
{
  uint8_t seed_result = 1;

  switch(seed_gen_s)
  {
    case SEED_GEN_NONE_S:
      
      break;
	  
    case SEED_GEN_INIT_S:
      seed_gen_init_disp(*lang);
      seed_gen_s = SEED_GEN_INIT_DISP_S;
      break;
	  
    case SEED_GEN_INIT_DISP_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_SEL_NOTI_INIT_S;
      }
      break;
	  
    case SEED_GEN_SEL_NOTI_INIT_S:
      seed_gen_set_word_disp(*lang);
      seed_gen_s = SEED_GEN_SEL_NOTI_S;
      break;

	  
    case SEED_GEN_SEL_NOTI_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_SEL_DISP_INIT_S;
      }
      break;

    case SEED_GEN_SEL_DISP_INIT_S:
      seed_gen_word_count_disp();
      seed_gen_s = SEED_GEN_SEL_DISP_S;
      break;
	  
    case SEED_GEN_SEL_DISP_S:
      seed_gen_word_count_sel(evt_mask);
      break;
	  
    case SEED_GEN_WORDS_INIT_DISP_S:
      mnemonic = (char *)mnemonic_generate(words_count);

      seed_gen_words_disp_init();
      seed_gen_s = SEED_GEN_WORDS_DISP_S;
      break;
	  
    case SEED_GEN_WORDS_DISP_S:
      seed_gen_word_disp(evt_mask, *lang);
      break;

    case SEED_GEN_FINAL_CHECK_INIT_S:
      seed_gen_final_check_init();
      break;

    case SEED_GEN_FINAL_CHECK_PPHRASE_INIT_S:
      pphrase_set_state(PPHRASE_INIT_S);
      seed_gen_s = SEED_GEN_FINAL_CHECK_PPHRASE_S;
      break;
      
    case SEED_GEN_FINAL_CHECK_PPHRASE_S:
      pphrase_confirm_state(evt_mask, lang);
      if((*evt_mask & CONFIRM_PPHRASE_EVENT) == CONFIRM_PPHRASE_EVENT)
      {
        verified_pphrase = 1;
        *evt_mask &= ~CONFIRM_PPHRASE_EVENT;
        seed_gen_s = SEED_GEN_FINAL_CHECK_WORD_INTRO_INIT_S;
      }
      if((*evt_mask & MISMATCH_PPHRASE_EVENT) == MISMATCH_PPHRASE_EVENT)
      {
        verified_pphrase = 0;
        *evt_mask &= ~MISMATCH_PPHRASE_EVENT;
        seed_gen_s = SEED_GEN_FINAL_FAIL_INIT_S;
      }
      break;

    case SEED_GEN_FINAL_CHECK_WORD_INTRO_INIT_S:
      seed_gen_final_check_words_intro(*lang);
      seed_gen_s = SEED_GEN_FINAL_CHECK_WORD_INTRO_S;
      break;
      
    case SEED_GEN_FINAL_CHECK_WORD_INTRO_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_INIT_S;
      }
      break;

    case SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_INIT_S:
      seed_gen_final_noti_word_init(*lang);
      seed_gen_s = SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_S;
      break;
      
    case SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_FINAL_CHECK_WORD_INIT_S;
      }
      break;
      
    case SEED_GEN_FINAL_CHECK_WORD_INIT_S:
      seed_gen_final_words_disp_init();
      seed_gen_s = SEED_GEN_FINAL_CHECK_WORD_S;
      break;
      
    case SEED_GEN_FINAL_CHECK_WORD_S:
      seed_gen_final_words_disp(evt_mask, *lang);
      break;

    case SEED_GEN_FINAL_FAIL_INIT_S:
      seed_gen_final_check_fail_init(*lang);
      seed_gen_s = SEED_GEN_FINAL_FAIL_S;
      break;
      
    case SEED_GEN_FINAL_FAIL_S:
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
        if(verified_pphrase)
        {
          seed_gen_s = SEED_GEN_FINAL_CHECK_WORD_INTRO_INIT_S;
        }
        else
        {
          seed_gen_s = SEED_GEN_FINAL_CHECK_INIT_S;
        }
      }      
      else if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_NONE_S;
        *evt_mask |= SEED_GENERATED_EVENT;
      }
      break;

    case SEED_GEN_FINAL_OK_INIT_S:
      seed_gen_final_check_ok_init(*lang);
      seed_gen_s = SEED_GEN_FINAL_OK_S;
      break;
      
    case SEED_GEN_FINAL_OK_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_SEED_GENERATE_S;
      }
      break;

    case SEED_GEN_SEED_GENERATE_S:
      if(mnemonic != NULL)
      {
        seed_gen_create_seed(*lang);
        seed_gen_s = SEED_GEN_SEND_APDU_S;
      }
      else
      {
        screen_clear(1);
        screen_disp_str(0,15, "error !!!!!!!!", &Font_7x10);
      }
      break;

    case SEED_GEN_MAX_RETRY_FAIL_INIT_S:
      seed_gen_max_retry_fail_init(*lang);
      seed_gen_s = SEED_GEN_MAX_RETRY_FAIL_S;
      break;

    case SEED_GEN_MAX_RETRY_FAIL_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_NONE_S;
        *evt_mask |= SEED_MAX_RETRY_EVENT;
      }
      break;
      
    case SEED_GEN_SEND_APDU_S:
      seed_result = APDU_set_init_seed(new_seed, 64);
      seed_gen_create_completed(*lang, seed_result);
	  
      flash_fd_has_seed(seed_result ? 1:0);
      dev_feature.has_seed = seed_result ? 1:0;

      seed_gen_s = SEED_GEN_COMPLETE_S;
      break;
	  
    case SEED_GEN_COMPLETE_S:
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
        seed_gen_s = SEED_GEN_NONE_S;
        *evt_mask |= SEED_GENERATED_EVENT;
      }
      break;
	  
    default:
      break;
  
  }
}


