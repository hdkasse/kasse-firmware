/**
  ******************************************************************************
  * @file    msg_dispatch.c
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


#include "msg_dispatch.h"
#include "screen_disp.h"
#include "user_type.h"

#define D_CRC_VALIDATION_CHECK
#ifdef D_CRC_VALIDATION_CHECK
#include "crc16.h"
#endif  

extern uint32_t event_mask;
extern received_raw_msg_callback_t rcv_raw_msg_cb;
extern void raw_message_callback(uint8_t *msg, uint32_t len);

#ifdef D_CRC_VALIDATION_CHECK
typedef struct
{
  hid_raw_msg_type msg;
  uint16_t crc_value;
}crc_inclue_data_type;
#endif

void usb_rx_message(UsbMessage *msg)
{
  if(rcv_raw_msg_cb)
  {
    rcv_raw_msg_cb(msg->message, msg->len);
  }
}

void usb_tx_message(uint8_t *msg, uint32_t len)
{
  usb_tx_helper(msg, len);
}


void usb_msg_init(void)
{
  usb_set_rx_callback(usb_rx_message);
  rcv_raw_msg_cb = raw_message_callback;
}

uint32_t raw_message_dispatch(uint8_t *msg, hid_raw_msg_type *raw_msg)
{
  hid_raw_msg_type *m;

#ifdef D_CRC_VALIDATION_CHECK
  crc_inclue_data_type *crc_data;
  uint16_t crc;

  crc_data = (crc_inclue_data_type *)msg;

  crc = crc16_ccitt(msg, sizeof(hid_raw_msg_type));

  if(crc_data->crc_value != crc)
  {
    memset(raw_msg, 0x0, sizeof(hid_raw_msg_type));

    raw_msg->msg_id = crc_data->msg.msg_id;
    raw_msg->sub_id = CRC_ERROR; // CRC error

    usb_tx_message((uint8_t*)raw_msg, sizeof(hid_raw_msg_type));
	return NONE_MSG_ID;
  }

#endif  

  m = (hid_raw_msg_type *)msg;


  do
  {
    memset(raw_msg, 0x0, sizeof(hid_raw_msg_type));
	
    raw_msg->msg_id = m->msg_id;
    raw_msg->sub_id = m->sub_id;
    raw_msg->data_len = m->data_len;
    memcpy(raw_msg->data, m->data, DATA_MAX_SIZE);
	
  }while(0);

  return raw_msg->msg_id;  
}

void send_raw_message(hid_raw_msg_type* raw_msg)
{
  hid_raw_msg_type m;
  memcpy(&m, raw_msg, sizeof(hid_raw_msg_type));
  usb_tx_message((uint8_t *)&m, sizeof(hid_raw_msg_type));

}


uint8_t apdu_msg_prepare
(
  hid_data_buff_type   *apdu_buff, 
  hid_raw_msg_type     *raw_msg
)
{
  uint8_t tr_apdu = 0;
  
  hid_raw_msg_type *m = (hid_raw_msg_type *)raw_msg;
  hid_data_buff_type *buff = (hid_data_buff_type *)apdu_buff;

  do
  {
    if((APDU_BUFF_MAX - buff->offset) < m->data_len)
    {
      m->sub_id = 0xFD; // message length overflow...
      break;
    }
	
    memcpy(&buff->data[buff->offset], m->data, m->data_len);
	
    if(m->sub_id == 0x11) // continuous apdu data
    {
      buff->offset += DATA_MAX_SIZE;
	  
	  if(buff->offset == APDU_BUFF_MAX)
      {
        tr_apdu = 1;
		buff->cont_flag = 1;
      }
	  
	  m->sub_id = 0x11;
	  
    }
	else if(m->sub_id == 0x10) // end of apdu data 
    {
      tr_apdu = 1;
	  buff->offset += m->data_len;
	  buff->cont_flag = 0;
	  m->sub_id = 0x10;
    }
	else
    {
      tr_apdu = 1;
	  buff->offset = m->data_len;
	  buff->cont_flag = 0;
	  m->sub_id = 0;
    }
	
  }while(0);

  return tr_apdu;  
}

void get_device_sw_version(hid_raw_msg_type *raw_msg)
{
  uint8_t ver_str[30];
  memset(ver_str, 0x0, sizeof(ver_str));
  strcpy(ver_str, BUILD_VER);
  strcat(ver_str, "/");
  strcat(ver_str, BUILD_DATE);
  
  raw_msg->msg_id = GET_VERSION_ID;
  raw_msg->sub_id = 0;
  raw_msg->data_len = strlen(ver_str);

  strcpy(raw_msg->data, ver_str);

  usb_tx_message((uint8_t*)raw_msg, sizeof(hid_raw_msg_type));
  
}

