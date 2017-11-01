/**
  ******************************************************************************
  * @file    pphrase.h
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
  
#ifndef __PPHRASE_H
#define __PPHRASE_H


#include <stdint.h>
#include "user_type.h"

typedef enum
{
  PPHRASE_NONE_S            = 0x0,
  PPHRASE_INIT_S,
  PPHRASE_INIT_DISP_S,
  PPHRASE_INPUT_INIT_S,
  PPHRASE_INPUT_PASSPHRASE_S,
  PPHRASE_FINAL_INIT_S,
  PPHRASE_FINAL_DISP_S,

  PPHRASE_S_MAX
}pphrase_state_type;

char *pphrase_get_passphrase(void);
void pphrase_set_state(pphrase_state_type state);
void pphrase_state(uint32_t *evt_mask, uint8_t *lang, session_state_type *next_state);
void pphrase_confirm_state(uint32_t *evt_mask, uint8_t *lang);

#endif/* __PPHRASE_H */

