/**
  ******************************************************************************
  * @file    flash_dload.h
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
#ifndef __DEVCONFIG_H_
#define __DEVCONFIG_H_

#include <stdint.h>

typedef enum
{
  DEVCFG_NONE_S            = 0x0,

  DEVCFG_INIT_S,
  DEVCFG_INIT_DISP_S,

  DEVCFG_START_NOTI_INIT_S,
  DEVCFG_START_NOTI_S,

  DEVCFG_SEL_CFG_INIT_S,
  DEVCFG_SEL_CFG_S,
  
  DEVCFG_S_MAX
}devcfg_state_type;

void devcfg_set_state(devcfg_state_type state);
void devcfg_state(uint32_t *evt_mask, uint8_t *lang, session_state_type *next_state);
uint8_t devcfg_get_cfg_mode(void);
void devcfg_set_cfg_mode(uint8_t cfg_mode);

#endif/* __DEVCONFIG_H_ */

