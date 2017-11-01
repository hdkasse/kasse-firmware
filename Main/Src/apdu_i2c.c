/**
  ******************************************************************************
  * @file    apdu_i2c.c
  * @author  Hyundai-pay Blockchain IOT Lab
  *          by jyhwang@hyundai-pay.com
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
  
#include "apdu_i2c.h"
#include "i2c.h"
//#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <string.h>

#include "msg_dispatch.h"
#include "user_type.h"
#include "flash_dload.h"

/*================ Define ====================*/
#ifndef OK
#define OK 1
#endif

#ifndef NOK
#define NOK 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define MAX_APPLET_NUM 4
#define APPLET_AID_LEN 7
#define CMD_TIMEOUT 0x75

typedef enum {
  APDU_CLA = 0,
  APDU_INS = 1,
  APDU_P1 = 2,
  APDU_P2 = 3,
  APDU_LC = 4,
  APDU_DATA = 5,
  MAX_APDU_CMD
} apdu_cmd_data_type;

typedef enum {
  APDU_ATR = 0x76,
  APDU_GET_TIMEOUT = 0x75,
  APDU_DELETE_KEY = 0xE4,
  APDU_EXTERNAL_AUTH = 0x82,
  APDU_GET_DATA = 0xCA,
  APDU_GET_STATUS = 0xF2,
  APDU_INIT_UPDATE = 0x50,
  APDU_MANAGE_CH = 0x70,
  APDU_PUT_DES_KEY = 0xD8,
  APDU_PUT_AES_KEY = APDU_PUT_DES_KEY,
  APDU_SELECT_FILE = 0xA4,
  APDU_SET_STATUS = 0xF0,
  APDU_STORE_DATA = 0xE2,
  APDU_INSTALL = 0xE6,
  APDU_LOAD = 0xE8,
  APDU_DO_SELF = 0x11,
  APDU_UPDATE = 0x6D,
  APDU_LASTDATA = 0x6E,
  APDU_STATUS_MAX = 0xFF
} apdu_cmd_ins_type;
apdu_cmd_ins_type apdu_status;

typedef enum {
  ERROR_UNKOWN = 0x00,
  ERROR_NONE = 0x01,
  ERROR_SW1SW2 = 0x02,
  ERROR_ACCESS = 0x03,
  ERROR_UNDIFINED_INS = 0xF1,
  ERROR_I2C_HAL = 0x6B,
  ERROR_MAX = 0xFF
} apdu_res_error_type;

typedef struct {
  uint8_t resp_rem;
  uint8_t resp_rem_len;
  uint8_t resp_rem_data[20];
} apdu_resp_remain_type;
static apdu_resp_remain_type remain_resp;
static uint8_t applet_aid_list[MAX_APPLET_NUM+1][APPLET_AID_LEN] = {0,};

/*================ Variable ====================*/

APDU_resp_length_type apdu_resp_length;

APDU_command_type cmd_buf;
APDU_response_type res_buf;
static uint8_t apdu_hid_process = 0;
static uint8_t applet_idx = (MAX_APPLET_NUM-1);

/*================ FUNCTION ====================*/
static uint8_t APDU_set_aid_info(uint8_t *data);

extern device_feature_type dev_feature;
extern hid_apdu_callback_t hid_apdu_callback;
void APDU_hid_send_apdu(void *msg)
{
  hid_raw_msg_type *m;
  hid_raw_msg_type resp;

  memset(&resp, 0x0, sizeof(hid_raw_msg_type));
  m = (hid_raw_msg_type *)msg;

  if(m->msg_id == A_UPDATE_ID)
  {
    APDU_set_pin_check(APDU_PIN_UPDATE, dev_feature.pin_status.pin_code, dev_feature.pin_status.pin_len, NULL);

    flash_set_sec_extra_ver((uint32_t)m->data[0]);
    flash_fd_has_seed(0);
    flash_fd_assign_pphrase(0);
    dev_feature.has_seed = 0;
    dev_feature.assign_pphrase = 0;
    
    resp.msg_id = A_UPDATE_ID;
    usb_tx_message((uint8_t*)&resp, sizeof(hid_raw_msg_type));
    return;
  }
}

// apud callback func.
extern apdu_rx_callback_t apdu_rx_callback;
void APDU_received_from_hid(void *msg)
{
  hid_raw_msg_type m;
  hid_data_buff_type *data = (hid_data_buff_type *)msg;

  memset(&m, 0x0, sizeof(hid_raw_msg_type));

  if(apdu_hid_process)
  {
    m.msg_id = APDU_MSG_ID;
    m.sub_id = APDU_ERROR;
    usb_tx_message((uint8_t*)&m, sizeof(hid_raw_msg_type));
    return;
  }

  apdu_hid_process = 1;
  APDU_data_process((uint8_t*)data, (uint8_t*)&m);

  m.msg_id = APDU_MSG_ID;
  usb_tx_message((uint8_t*)&m, sizeof(hid_raw_msg_type));
  apdu_hid_process = 0;
}


#if 0
void * APDU_memcpy(void *dst, void *src, uint8_t n)
{
  uint8_t *d =dst, *s = src;
  while(n--)
    *d++ = *s++;
  return dst;
}
#endif
void APDU_DELAY(uint32_t delay)
{
  HAL_Delay(delay);
}

void APDU_reset(void)
{
  apdu_rx_callback = APDU_received_from_hid;
  hid_apdu_callback = APDU_hid_send_apdu;
  
  HAL_GPIO_WritePin(SECURE_RESETn_GPIO_Port, SECURE_RESETn_Pin, GPIO_PIN_RESET);
  APDU_DELAY(10);
  HAL_GPIO_WritePin(SECURE_RESETn_GPIO_Port, SECURE_RESETn_Pin, GPIO_PIN_SET);
  APDU_DELAY(10);
}

HAL_StatusTypeDef APDU_write_specific_command(uint8_t cmd)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  status = HAL_I2C_Master_Transmit(&hi2c1,SECURE_I2C_ADDR, &cmd,1,10);
  if(status == HAL_OK)
  {
    //
  }
  else
  {
    //error
  }
  return status;
}

uint16_t ADPU_read_timeout_resopnse(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint16_t timeout=0;

  status = APDU_read_resp_length_info();
  if(status == HAL_OK)
  {
    timeout = ((apdu_resp_length.MSB <<8) | apdu_resp_length.LSB);
  }
  else
  {
    //error
  }

  return timeout;
}

HAL_StatusTypeDef APDU_write_cmd_len(uint8_t *pData, uint32_t size)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  status = HAL_I2C_Master_Transmit(&hi2c1,SECURE_I2C_ADDR,pData,size,100);
  if(status == HAL_OK)
  {
    //
  }
  else
  {
    //
  }
  return status;
}

HAL_StatusTypeDef APDU_read_response_data(uint8_t *pData, uint8_t size)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  status = HAL_I2C_Master_Receive(&hi2c1,SECURE_I2C_ADDR,pData,size,100);

  if(status == HAL_OK)
  {
    //
  }
  else
  {
    //
  }
  return status;
}


apdu_cmd_ins_type APDU_get_local_status(void)
{
  return apdu_status;
}

void APDU_set_local_status(uint8_t ins)
{
  apdu_status = (apdu_cmd_ins_type)ins;
}

HAL_StatusTypeDef APDU_read_resp_length_info(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint8_t count = 0;

  apdu_resp_length.MSB = 0;
  apdu_resp_length.LSB = 0;

  do
  {
    status = HAL_I2C_Master_Receive(&hi2c1,SECURE_I2C_ADDR,(uint8_t*)&apdu_resp_length,2,100);
    if(++count>300) break;
  } while(apdu_resp_length.LSB==0);

  if(status == HAL_OK)
  {
    //OK
  }
  else
  {
    //Fail
  }

  return status;
}

HAL_StatusTypeDef APDU_get_timeout_data(uint8_t *rsp)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint16_t timeout_val;

  status = APDU_write_specific_command(CMD_TIMEOUT);

  if(status == HAL_OK)
  {
    status = APDU_read_resp_length_info();
    if(status == HAL_OK)
    {
      if((apdu_resp_length.LSB & 0x0F) == 0x02)
      {
        timeout_val = ADPU_read_timeout_resopnse();
        rsp[0] = (timeout_val & 0xFF00) >> 8;
        rsp[1] = (timeout_val & 0xFF);
      }
    }
  }

  return status;
}

HAL_StatusTypeDef APDU_test_command(uint8_t *rsp)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint8_t test_size;

  status = APDU_write_specific_command(0x76);
  if(status == HAL_OK)
  {
    status = APDU_read_resp_length_info();
    if(status == HAL_OK)
    {
      test_size = (apdu_resp_length.LSB & 0x0FF);
      status = APDU_read_response_data((uint8_t*)rsp,test_size);
    }
  }
  else
  {
  }

  return status;
}

uint16_t ADPU_command_data_size(uint8_t lc_size)
{
  return (sizeof(APDU_cmd_header_type) + lc_size);
}

HAL_StatusTypeDef APDU_commad_response(uint8_t *cmd, uint8_t *rsp)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  APDU_command_type *apdu_cmd = (APDU_command_type *)cmd;
  uint8_t data_len,cmd_size;

  cmd_size = sizeof(APDU_cmd_header_type) + apdu_cmd->header.Lc;

  if(apdu_cmd->header.INS == 0x70)
  {
    cmd_size = 4;
  }
  status = APDU_write_cmd_len((uint8_t*)cmd, cmd_size);

  if(status == HAL_OK)
  {    
    status = APDU_read_resp_length_info(); 
    if(status == HAL_OK)
    {
      data_len = apdu_resp_length.LSB;
      APDU_read_response_data((uint8_t*)rsp, data_len);
    }
  }
  return status;
}

HAL_StatusTypeDef APDU_commad_size_response(uint8_t *cmd, uint8_t size, uint8_t *rsp)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  status = APDU_write_cmd_len((uint8_t*)cmd, size);

  if(status == HAL_OK)
  {    
    //APDU_DELAY(100);
    status = APDU_read_response_data((uint8_t*)rsp, 2);
  }
  return status;
}

apdu_res_error_type APDU_check_sw12(uint8_t sw1, uint8_t sw2)
{
  apdu_res_error_type result = ERROR_SW1SW2;

  if((sw1==0x90) && (sw2==0x00))
  {
    result = ERROR_NONE;
  }

  return result;
}

void APDU_save_applet_aids(uint8_t *msg)
{
  uint8_t i;
  uint8_t temp[7] = {0,};

  if(msg[0] == 0xA0) return;

  for(i=0; i<MAX_APPLET_NUM; i++)
  {
    if(!memcmp(applet_aid_list+i, msg, sizeof(temp)))
    {
      applet_idx = i;
      break;
    }
    if(!memcmp(applet_aid_list+i, temp, sizeof(temp)))
    {
      applet_idx = i;
      memcpy(applet_aid_list+applet_idx, msg, APPLET_AID_LEN);
      break;
    }
  }

}

uint8_t APDU_HID_data_rx_process(uint8_t *in_data)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  hid_raw_msg_type *hid_msg = (hid_raw_msg_type *)in_data;
  apdu_cmd_ins_type support_cmd = APDU_STATUS_MAX;
  uint8_t data_in_len, result = NOK;

  if(hid_msg == NULL)
  {
    return result;
  }

  if(hid_msg->msg_id != APDU_MSG_ID)
  {
    return result;
  }

  memset((void*)&cmd_buf, 0x00, sizeof(APDU_command_type));
  memset((void*)&res_buf, 0x00, sizeof(APDU_response_type));

  cmd_buf.header.CLA = hid_msg->data[APDU_CLA];
  cmd_buf.header.INS = hid_msg->data[APDU_INS];
  cmd_buf.header.P1 = hid_msg->data[APDU_P1];
  cmd_buf.header.P2 = hid_msg->data[APDU_P2];
  cmd_buf.header.Lc = hid_msg->data[APDU_LC];
  data_in_len = cmd_buf.header.Lc;
  support_cmd = (apdu_cmd_ins_type)cmd_buf.header.INS;

  switch(support_cmd)
  {
    case APDU_ATR:
      status = APDU_test_command((uint8_t*)&res_buf.Data[0]);
      break;

    case APDU_GET_TIMEOUT:
      status = APDU_get_timeout_data((uint8_t*)&res_buf.Data[0]);
      break;

    case APDU_MANAGE_CH:
    case APDU_GET_DATA:
    case APDU_SELECT_FILE:
    case APDU_INIT_UPDATE:
    case APDU_GET_STATUS:
    case APDU_EXTERNAL_AUTH:
    case APDU_INSTALL:
    case APDU_LOAD:
    case APDU_DELETE_KEY:
    case APDU_DO_SELF:
      if(data_in_len > 0)
      {
        for(uint8_t i=0; i<data_in_len; i++)
        {
          cmd_buf.body.Data[i] = hid_msg->data[i+APDU_DATA];
        }
      }
      if((hid_msg->data_len & 0x00FF) == ADPU_command_data_size(cmd_buf.header.Lc))
      {
        status = APDU_commad_response((uint8_t*)&cmd_buf, (uint8_t*)&res_buf.Data[0]);
      }
      break;

    default:
      break;
  }

  if(status == HAL_OK)
  {
    APDU_set_local_status(cmd_buf.header.INS);
    result = OK;
  }

  return result;
}

void APDU_HID_data_tx_process(uint8_t *tx_buf)
{
  hid_raw_msg_type *HID_tx_data = (hid_raw_msg_type *)tx_buf;
  APDU_response_type *APDU_res;
  apdu_cmd_ins_type apdu_local_status = APDU_STATUS_MAX;
  uint8_t data_len;
  uint16_t total_data_len;

  HID_tx_data->msg_id = APDU_MSG_ID;
  HID_tx_data->sub_id = APDU_MSG_ID+1;

  APDU_res = &res_buf;
  apdu_local_status = APDU_get_local_status();

  switch(apdu_local_status)
  {
    case APDU_ATR:
      data_len = apdu_resp_length.LSB;
      break;

    case APDU_GET_TIMEOUT:
      data_len = 0x2; //timeout_val
      break;

    case APDU_MANAGE_CH:
    case APDU_GET_DATA:
    case APDU_SELECT_FILE:
    case APDU_INIT_UPDATE:
    case APDU_GET_STATUS:
    case APDU_EXTERNAL_AUTH:
    case APDU_INSTALL:
    case APDU_LOAD:
    case APDU_DELETE_KEY:
    case APDU_DO_SELF:
      data_len = apdu_resp_length.LSB;
      break;
      
    default:
      data_len = 0;
      break;
  }

  total_data_len = data_len;
  HID_tx_data->data_len = total_data_len;

  memcpy((uint8_t*)&HID_tx_data->data[0], (uint8_t*)&APDU_res->Data[0], data_len);
}

uint8_t APDU_data_process(uint8_t *in_data, uint8_t *out_data)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  hid_data_buff_type *hid_msg = (hid_data_buff_type *)in_data;
  hid_raw_msg_type *HID_tx_data = (hid_raw_msg_type *)out_data;
  apdu_cmd_ins_type support_cmd = APDU_STATUS_MAX;
  uint8_t data_in_len, data_out_len, result = NOK;
  uint8_t rsp_buf[90] = {0,};
  uint16_t total_out_len;

  if(hid_msg == NULL)
  {
    return result;
  }

  memset((void*)&cmd_buf, 0x00, sizeof(APDU_command_type));
  memset(rsp_buf, 0x0, ARRY_SIZE(rsp_buf));
  cmd_buf.header.CLA = hid_msg->data[APDU_CLA];
  cmd_buf.header.INS = hid_msg->data[APDU_INS];
  cmd_buf.header.P1 = hid_msg->data[APDU_P1];
  cmd_buf.header.P2 = hid_msg->data[APDU_P2];
  cmd_buf.header.Lc = hid_msg->data[APDU_LC];
  data_in_len = cmd_buf.header.Lc;

  if(data_in_len > APDU_CMD_DATA_LEN)
    data_in_len = APDU_CMD_DATA_LEN;

  data_out_len = 0;
  support_cmd = (apdu_cmd_ins_type)cmd_buf.header.INS;

  switch(support_cmd)
  {
    case APDU_ATR:
      status = APDU_test_command((uint8_t*)&HID_tx_data->data[0]);
      data_out_len = apdu_resp_length.LSB;
      break;

    case APDU_GET_TIMEOUT:
      status = APDU_get_timeout_data(rsp_buf);
      data_out_len = 0x2; //timeout_val
      break;

    case APDU_MANAGE_CH:
    case APDU_GET_DATA:
    case APDU_SELECT_FILE:
    case APDU_INIT_UPDATE:
    case APDU_GET_STATUS:
    case APDU_EXTERNAL_AUTH:
    case APDU_INSTALL:
    case APDU_LOAD:
    case APDU_DELETE_KEY:
    case APDU_DO_SELF:
      memcpy((void*)cmd_buf.body.Data, (void*)(hid_msg->data+APDU_DATA), data_in_len);
      if((support_cmd==APDU_SELECT_FILE) && (cmd_buf.body.Data[data_in_len-1]==0x91)) break;
      status = APDU_commad_response((uint8_t*)&cmd_buf, rsp_buf);
      data_out_len = apdu_resp_length.LSB;
      break;

    case APDU_LASTDATA:
      if(remain_resp.resp_rem)
      {
        status = HAL_OK;
        data_out_len = remain_resp.resp_rem_len;
        memcpy(rsp_buf, remain_resp.resp_rem_data, data_out_len);
      }
      break;

    default:
      if((support_cmd==(*(*(applet_aid_list+MAX_APPLET_NUM)+4)))||(support_cmd==(*(*(applet_aid_list+MAX_APPLET_NUM)+1))))break;
      memcpy((void*)cmd_buf.body.Data, (void*)(hid_msg->data+APDU_DATA), data_in_len);
      status = APDU_commad_response((uint8_t*)&cmd_buf, rsp_buf);
      data_out_len = apdu_resp_length.LSB;
      break;
  }

  if(status == HAL_OK)
  {
    if(support_cmd == APDU_ATR)
    {
      result = OK;
    }
    else
    {
      if(data_out_len > DATA_MAX_SIZE)
      {
        memcpy(HID_tx_data->data, rsp_buf, DATA_MAX_SIZE);
        remain_resp.resp_rem = 1;
        remain_resp.resp_rem_len = (data_out_len-DATA_MAX_SIZE);
        memcpy(remain_resp.resp_rem_data, rsp_buf+56, remain_resp.resp_rem_len);
        HID_tx_data->sub_id = APDU_MSG_ID+1;
      }
      else
      {
        memcpy(HID_tx_data->data, rsp_buf, data_out_len);
        memset((void*)&remain_resp, 0x0, sizeof(apdu_resp_remain_type));
      }
      result = APDU_check_sw12(HID_tx_data->data[data_out_len-2],HID_tx_data->data[data_out_len-1]);
      if((support_cmd==APDU_SELECT_FILE) && (result==ERROR_NONE))
      {
        APDU_save_applet_aids(&cmd_buf.body.Data[0]);
      }
    }
    total_out_len = data_out_len;
    HID_tx_data->data_len = total_out_len;
  }
  else
  {
    data_out_len = 2;
    HID_tx_data->data_len = data_out_len;
    HID_tx_data->data[0] = ERROR_I2C_HAL;
  }

  return result;
}

static apdu_res_error_type APDU_process_internal(uint8_t *in_buf, uint8_t enable)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  apdu_res_error_type result=ERROR_UNKOWN;
  uint8_t data_in_len,data_out_len;

  if((apdu_hid_process == 1) && !enable)
  {
    return ERROR_ACCESS;
  }

  memset((void*)&cmd_buf, 0x00, sizeof(APDU_command_type));
  memset((void*)&res_buf, 0x00, sizeof(APDU_response_type));
  
  cmd_buf.header.CLA = in_buf[APDU_CLA];
  cmd_buf.header.INS = in_buf[APDU_INS];
  cmd_buf.header.P1 = in_buf[APDU_P1];
  cmd_buf.header.P2 = in_buf[APDU_P2];
  cmd_buf.header.Lc = in_buf[APDU_LC];
  data_in_len = cmd_buf.header.Lc;

  if(data_in_len > 0)
  {
    for(uint8_t i=0; i<data_in_len; i++)
    {
      cmd_buf.body.Data[i] = in_buf[APDU_DATA+i];
    }
  }

  status = APDU_commad_response((uint8_t*)&cmd_buf, (uint8_t*)&res_buf.Data[0]);
  data_out_len = apdu_resp_length.LSB;

  if(status == HAL_OK)
  {
    result = APDU_check_sw12(res_buf.Data[data_out_len-2],res_buf.Data[data_out_len-1]);
    if((applet_aid_list[MAX_APPLET_NUM][0] == 0x00) && (cmd_buf.header.INS==0xA4) 
       && (cmd_buf.body.Data[0] != 0xA0) && (result==ERROR_NONE))
    {
      memcpy(applet_aid_list+MAX_APPLET_NUM, res_buf.Data, (data_out_len-2));
    }
  }
  else
  {
    result = ERROR_I2C_HAL;
  }

  return result;
}

uint8_t APDU_command_test(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint8_t result = NOK;
  uint8_t test_val[] = {0x3F,0x3B,0xF8,0x96,0x00,0x00,0x80,0x11,0xFE,0x00,0x73,
                        0xC8,0x40,0x00,0x00,0x90,0x00,0x6A,0x2E,0x3D,0x4C,0x5B};
  uint8_t read[0x11] = {0,};

  if(apdu_hid_process == 1)
  {
    return result+2;
  }

  status = APDU_test_command(read);
  if(status == HAL_OK)
  {
    if(!memcmp(read, test_val+1, ARRY_SIZE(read)))
    {
      result = OK;
    }
  }
  return result;
}

uint8_t APDU_set_init_seed(uint8_t *msg, uint8_t msg_len)
{
  apdu_res_error_type result=ERROR_UNKOWN;
  uint8_t i,temp_num,idx=0;
  uint8_t send_buf[70],temp_buf[APPLET_AID_LEN*3];

  temp_num = APDU_set_aid_info(temp_buf);

  for(i=0; i<temp_num; i++)
  {
    idx = 0;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APDU_SELECT_FILE;
    send_buf[idx++] = 0x04;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APPLET_AID_LEN;
    memcpy(send_buf+idx, applet_aid_list+i, APPLET_AID_LEN);
    result = APDU_process_internal(send_buf, 0);
  
    if(result != ERROR_NONE)
    {
     continue; //error display?
    }

    idx = 0;
    send_buf[idx++] = 0x80;
    send_buf[idx++] = (uint8_t)*(*(applet_aid_list+MAX_APPLET_NUM)+0);
    send_buf[idx++] = 0x00;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = msg_len;
    memcpy(send_buf+idx, msg, msg_len);
    result = APDU_process_internal(send_buf, 0);
  }

  return result;
}

uint8_t APDU_set_factofy_init(void)
{
  apdu_res_error_type result=ERROR_UNKOWN;
  uint8_t i,temp_num,idx=0;
  uint8_t send_buf[70], temp_buf[APPLET_AID_LEN*3];

  temp_num = APDU_set_aid_info(temp_buf);

  for(i=0; i<temp_num; i++)
  {
    idx = 0;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APDU_SELECT_FILE;
    send_buf[idx++] = 0x04;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APPLET_AID_LEN;
    memcpy(send_buf+idx, applet_aid_list+i, APPLET_AID_LEN);
    result = APDU_process_internal(send_buf, 0);

    idx = 0;
    send_buf[idx++] = 0x80;
    send_buf[idx++] = (uint8_t)*(*(applet_aid_list+MAX_APPLET_NUM)+2);
    send_buf[idx++] = 0x00;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = 0x00;
    result = APDU_process_internal(send_buf, 0);
  }

  return result;
}

uint8_t APDU_set_pin_check(APDU_pincheck_type operate, uint8_t* data, uint8_t data_len, uint8_t *out_data)
{
  apdu_res_error_type result=ERROR_UNKOWN;
  uint8_t idx=0;
  uint8_t send_buf[70], pin_result[2];

  if(applet_idx != 0)
  {
    idx = 0;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APDU_SELECT_FILE;
    send_buf[idx++] = 0x04;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APPLET_AID_LEN;
    memcpy(send_buf+idx, applet_aid_list, APPLET_AID_LEN);
    result = APDU_process_internal(send_buf, 0);
  }

  idx = 0;
  send_buf[idx++] = 0x80;
  send_buf[idx++] = (uint8_t)*(*(applet_aid_list+MAX_APPLET_NUM)+1);
  send_buf[idx++] = operate;
  send_buf[idx++] = 0x00;
  send_buf[idx++] = data_len;
  memcpy(send_buf+idx, data, data_len);
  result = APDU_process_internal(send_buf, 0);

  if(result == ERROR_NONE)
  {
    memcpy(pin_result, res_buf.Data,(apdu_resp_length.LSB-2));
    result = (pin_result[0] == 0x01) ? ERROR_NONE : ERROR_UNKOWN;
    if((apdu_resp_length.LSB > 3) && (out_data != NULL))
    {
      *out_data = pin_result[1];
    }
  }

  return result;
}

static uint8_t APDU_set_aid_info(uint8_t *data)
{
  apdu_res_error_type result=ERROR_UNKOWN;
  uint8_t idx=0,num,len,valid_num=1;
  uint8_t send_buf[20];

  if(applet_idx != 0)
  {
    idx = 0;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APDU_SELECT_FILE;
    send_buf[idx++] = 0x04;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APPLET_AID_LEN;
    memcpy(send_buf+idx, applet_aid_list, APPLET_AID_LEN);
    result = APDU_process_internal(send_buf, 0);
  }

  idx = 0;
  send_buf[idx++] = 0x80;
  send_buf[idx++] = (uint8_t)*(*(applet_aid_list+MAX_APPLET_NUM)+3);
  send_buf[idx++] = 0x00;
  send_buf[idx++] = 0x00;
  send_buf[idx++] = 0x00;
  result = APDU_process_internal(send_buf, 0);

  if(result == ERROR_NONE)
  {
    num = res_buf.Data[0];
    for(uint8_t j=0; j<num; j++)
    {
      len = res_buf.Data[1+j*8];
      memcpy(data+(j*APPLET_AID_LEN), res_buf.Data+(2+j*8), len);
      idx = 0;
      send_buf[idx++] = 0x00;
      send_buf[idx++] = APDU_SELECT_FILE;
      send_buf[idx++] = 0x04;
      send_buf[idx++] = 0x00;
      send_buf[idx++] = APPLET_AID_LEN;
      memcpy(send_buf+idx, data+(j*7), APPLET_AID_LEN);
      result = APDU_process_internal(send_buf, 0);
      if(result == ERROR_NONE)
      {
        APDU_save_applet_aids(data);
        valid_num++;
      }
    }
  }

  return valid_num;
}

uint8_t APDU_set_version(uint8_t *data)
{
  apdu_res_error_type result=ERROR_UNKOWN;
  uint8_t idx=0;
  uint8_t send_buf[20];

  if(applet_idx != 0)
  {
    idx = 0;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APDU_SELECT_FILE;
    send_buf[idx++] = 0x04;
    send_buf[idx++] = 0x00;
    send_buf[idx++] = APPLET_AID_LEN;
    memcpy(send_buf+idx, applet_aid_list, APPLET_AID_LEN);
    result = APDU_process_internal(send_buf, 1);
  }

  idx = 0;
  send_buf[idx++] = 0x80;
  send_buf[idx++] = (uint8_t)*(*(applet_aid_list+MAX_APPLET_NUM)+5);
  send_buf[idx++] = 0x00;
  send_buf[idx++] = 0x00;
  send_buf[idx++] = 0;
  result = APDU_process_internal(send_buf, 0);

  if((result == ERROR_NONE) && (data != NULL))
  {
    memcpy(data, res_buf.Data,(apdu_resp_length.LSB-2));
  }

  return result;
}
