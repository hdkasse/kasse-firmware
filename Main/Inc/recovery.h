/**
  ******************************************************************************
  * @file    recovery.h
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
  
#ifndef __RECOVERY_H
#define __RECOVERY_H


#include <stdint.h>

typedef enum
{
  RECOVERY_NONE_S            = 0x0,
  RECOVERY_INIT_S,
  RECOVERY_INIT_DISP_S,

  RECOVERY_WORD_CNT_NOTI_INIT_S,
  RECOVERY_WORD_CNT_NOTI_S,
  RECOVERY_WORD_CNT_SEL_INIT_S,
  RECOVERY_WORD_CNT_SEL_S,

  RECOVERY_WORD_SEL_INTRO_INIT_S,
  RECOVERY_WORD_SEL_INTRO_S,
  
  RECOVERY_WORD_INPUT_INIT_S,
  RECOVERY_WORD_INPUT_S,
  
  RECOVERY_FULL_WORD_SEL_INIT_S,
  RECOVERY_FULL_WORD_SEL_S,

  RECOVERY_NO_MATCH_WORD_INIT_S,
  RECOVERY_NO_MATCH_WORD_S,

  RECOVERY_INPUT_COMPLETE_INIT_S,
  RECOVERY_INPUT_COMPLETE_S,

  RECOVERY_SEED_GENERATE_S,
  RECOVERY_SEND_APDU_S,
  RECOVERY_COMPLETE_S,
  
  RECOVERY_S_MAX
}recovery_state_type;

void seed_recovery_set_state(recovery_state_type state);
void seed_recovery_state(uint32_t *evt_mask, uint8_t *lang);


#endif/* __RECOVERY_H */
