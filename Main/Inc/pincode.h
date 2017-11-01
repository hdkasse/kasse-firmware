
/**
  ******************************************************************************
  * @file    pincode.h
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
#ifndef __PINCODE_H_
#define __PINCODE_H_


typedef enum
{
  OWNER_NONE            = 0x0,
  OWNER_FACTORY_INIT,

  PIN_OWNER_MAX
}pin_check_owner_type;

typedef enum
{
  PIN_NONE_STATE       = 0x0,

  PIN_VERIFY_INIT,
  PIN_VERIFY,
  PIN_VERIFY_INVALID,
  PIN_VERIFY_PERM_FAIL,
  PIN_VERIFY_OK,

  PIN_EMPTY_INIT,
  PIN_EMPTY_NOTI,

  PIN_GENERATE_INIT,
  PIN_GENERATE,
  PIN_GENERATE_FAIL,
  PIN_1ST_GEN_FAIL,
  PIN_GENERATE_OK,
  PIN_SHORT_INPUT_INIT,
  PIN_SHORT_INPUT_NOTI,

  PIN_INPUT_INIT,
  PIN_GEN_1ST_DISP,
  PIN_GEN_2ND_DISP,
  PIN_INPUT_DISP,
  PIN_INPUT,

  PIN_FULL_REMOVE,
  
  PIN_STATE_MAX
}pin_state_type;

typedef enum
{
  NONE_PROCESS        = 0x00,
  PROC_PIN_VERIFY     = 0x01,
  PROC_PIN_1ST_GEN    = 0x02,
  PROC_PIN_GENERATE   = 0x03,
  PROC_CHANGE_PIN     = 0x04,
  MAX_PROCESS
}process_type;


uint8_t pincode_validation(void);

void pin_state_mach(uint32_t *evt_mask, uint8_t *lang);
uint8_t pincode_is_cached(void);
void pincode_set_state(pin_state_type state);

void pincode_set_type(process_type ptype);
process_type pincode_get_type(void);

void simple_pin_set_owner(pin_check_owner_type owner);
void simple_check_pin_init(uint8_t *lang);
void simple_check_pin(uint32_t *evt_mask, session_state_type *next_state, uint8_t is_submenu);

#endif/* __PINCODE_H_ */

