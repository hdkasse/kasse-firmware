/**
  ******************************************************************************
  * @file    seed_gen.h
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

#ifndef __SEED_GEN_H_
#define __SEED_GEN_H_

#include <stdint.h>

typedef enum
{
  SEED_GEN_NONE_S            = 0x0,

  SEED_GEN_INIT_S,
  SEED_GEN_INIT_DISP_S,
  SEED_GEN_SEL_NOTI_INIT_S,
  SEED_GEN_SEL_NOTI_S,
  SEED_GEN_SEL_DISP_INIT_S,
  SEED_GEN_SEL_DISP_S,
  SEED_GEN_WORDS_INIT_DISP_S,
  SEED_GEN_WORDS_DISP_S,

  SEED_GEN_FINAL_CHECK_INIT_S,
  SEED_GEN_FINAL_CHECK_PPHRASE_INIT_S,
  SEED_GEN_FINAL_CHECK_PPHRASE_S,
  SEED_GEN_FINAL_CHECK_WORD_INTRO_INIT_S,
  SEED_GEN_FINAL_CHECK_WORD_INTRO_S,

  SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_INIT_S,
  SEED_GEN_FINAL_CHECK_NOTI_RANDOM_WORD_S,
  
  SEED_GEN_FINAL_CHECK_WORD_INIT_S,
  SEED_GEN_FINAL_CHECK_WORD_S,

  SEED_GEN_FINAL_FAIL_INIT_S,
  SEED_GEN_FINAL_FAIL_S,

  SEED_GEN_FINAL_OK_INIT_S,
  SEED_GEN_FINAL_OK_S,

  SEED_GEN_MAX_RETRY_FAIL_INIT_S,
  SEED_GEN_MAX_RETRY_FAIL_S,
  
  SEED_GEN_SEED_GENERATE_S,

  SEED_GEN_COMPLETE_S,
  SEED_GEN_SEND_APDU_S,
  
  SEED_GEN_MAX
}seed_gen_state_type;

void seed_gen_set_state(seed_gen_state_type state);
void seed_gen_state(uint32_t *evt_mask, uint8_t *lang);

#endif/* __SEED_GEN_H_ */
