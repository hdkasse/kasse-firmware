/**
  ******************************************************************************
  * @file    msg_dispatch.h
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

#ifndef MSG_DISPATCH_H
#define MSG_DISPATCH_H

#include "usb_user.h"

#define DATA_MAX_SIZE 56

#define APDU_BUFF_MAX (DATA_MAX_SIZE * 5)


typedef enum
{
  NONE_MSG_ID         = 0x00,
  SET_DNLOAD_MODE     = 0x01,
  DNLOAD_MSG_ID       = 0x02,
  GET_VERSION_ID      = 0x03,
  GET_DEV_SN          = 0x04,
  PUT_DEV_SN          = 0x05,

  APDU_MSG_ID         = 0x10,
  PRE_SIGN_ID         = 0x11,
  AFT_SIGN_ID         = 0x12,

  A_UPDATE_ID         = 0x13,

  SEC_UPDATE_ID       = 0x14,
  
  HW_TEST_ID          = 0x20,

  MAX_MSG_ID
}msg_id_type;

typedef enum
{
  UPDATE_START        = 0x01,
  UPDATE_STOP         = 0x02,
  UPDATE_MAX
}sec_update_subid_type;

typedef enum
{
  NEED_PIN_VERIFY      = 0xF0,
  ERASE_PAGES_ERROR    = 0xF1,
  WRITE_DATA_ERROR     = 0xF2,
  ANY_ERROR            = 0xF3,
  APDU_ERROR           = 0xF4,
  USER_CANCEL          = 0xFC,
  INVALID_BOOT_MODE    = 0xFD,
  
  TYPE_MAX
}resp_err_sub_id_type;

typedef struct
{
  uint8_t    msg_id;
  uint8_t    sub_id;
  uint16_t   data_len;
  uint8_t    data[DATA_MAX_SIZE];
}hid_raw_msg_type;


typedef struct
{
  uint8_t  cont_flag;
  uint16_t offset;
  uint8_t  data[APDU_BUFF_MAX];
}hid_data_buff_type;

void usb_tx_message(uint8_t *msg, uint32_t len);

void usb_msg_init(void);

uint32_t raw_message_dispatch(uint8_t *msg, hid_raw_msg_type *raw_msg);
void send_raw_message(hid_raw_msg_type* raw_msg);

uint8_t apdu_msg_prepare
(
  hid_data_buff_type   *apdu_buff, 
  hid_raw_msg_type     *raw_msg
);

void get_device_sw_version(hid_raw_msg_type *raw_msg);
void handle_device_sn(hid_raw_msg_type *raw_msg, msg_id_type id);

#endif/* MSG_DISPATCH_H */
