
/**
  ******************************************************************************
  * @file    trans_noti.c
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

#ifndef __TRANS_NOTI_H_
#define __TRANS_NOTI_H_

#include <stdint.h>
#include "user_type.h"


typedef enum
{
  TRAN_NOTI_NONE_S            = 0x0,

  TRAN_PRE_SIGN_INIT_S,
  TRAN_PRE_SIGN_S,

  TRAN_CHECK_PIN_INIT_S,
  TRAN_CHECK_PIN_S,

  TRAN_AFT_SIGN_INIT_S,
  TRAN_AFT_SIGN_S,
  
  TRAN_NOTI_MAX
}tran_noti_state_type;

void tran_noti_set_state(tran_noti_state_type state);
void tran_noti_state(uint32_t *evt_mask, uint8_t *lang, session_state_type *next_state);

void sec_update_notify_init(uint8_t lang);

#endif/* __TRANS_NOTI_H_ */
