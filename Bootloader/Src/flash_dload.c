/**
  ******************************************************************************
  * @file    flash_dload.c
  * @author  Hyundai-pay Blockchain IOT Lab
  *          by nakjink@hyundai-pay.com
  ******************************************************************************
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


#include <string.h>
#include "flash_dload.h"
#include "user_type.h"
#include "screen_disp.h"
#include "res_img.h"

//#define D_DWONLOAD_PROG_STR

#define _4BYTE sizeof(uint32_t)
#define _2BYTE sizeof(uint16_t)

static dload_packet_type rcv_packet;
static uint32_t flash_wr_offset = 0;
static uint8_t flash_1st_addr[4] = {0, };

static dnload_state_type dnload_state = DNLOAD_NONE_S;

HAL_StatusTypeDef flash_erase_all_page(uint32_t total_size)
{
  HAL_StatusTypeDef status;

  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PAGEError;
  
  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
  EraseInitStruct.NbPages = (total_size % FLASH_PAGE_SIZE) ? (total_size/FLASH_PAGE_SIZE + 1) : (total_size/FLASH_PAGE_SIZE);

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

  if(status != HAL_OK)
  {
	// DEBUG Code....  TBD...
    //uint32_t error;
    //error = HAL_FLASH_GetError();
  }

  return status;
}

HAL_StatusTypeDef flash_dload_write(uint32_t offset, uint32_t len, uint8_t *data)
{
  HAL_StatusTypeDef status;
  
  uint32_t start = FLASH_USER_START_ADDR;
  uint32_t data_word[1];
  uint32_t i = 0;
  
  start += offset;
  
  /* Long word writes */
  for(i = 0 ; i < len/_4BYTE; i++)
  {
      memcpy(data_word, data, _4BYTE);
	  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, start, (uint32_t)*data_word);

      // check flash status register for error condition
      if(status != HAL_OK) {
          goto error;
      }
      start += _4BYTE;
      data += _4BYTE;
  }

  /* 
      ***** is below code need ?  ** by octopusy ***** 
      STM32F042X6 sample code don't have the type of FLASH_TYPEPROGRAM_BYTE
  */
  
  /* Byte write for last remaining bytes < longword */
  
error:
	
  return(status);
}

void start_resp_msg(uint8_t start)
{
  hid_raw_msg_type m;
  memset(&m, 0x0, sizeof(hid_raw_msg_type));
  m.msg_id = SET_DNLOAD_MODE;
  if(start)m.sub_id = 1;
  else m.sub_id = USER_CANCEL;
  	
  usb_tx_message((uint8_t *)&m, sizeof(hid_raw_msg_type));
}

void dnload_erase_page(hid_raw_msg_type *raw_msg)
{
  uint32_t image_size = 0;
  HAL_StatusTypeDef status;
  hid_raw_msg_type m;

  image_size = ((raw_msg->data[3] & 0xFF)<<24) +
               ((raw_msg->data[2] & 0xFF)<<16) +
               ((raw_msg->data[1] & 0xFF)<<8) +
               (raw_msg->data[0] & 0xFF);

  status = flash_erase_all_page(image_size);

  memset(&m, 0x0, sizeof(hid_raw_msg_type));
  m.msg_id = DNLOAD_MSG_ID;
  if(status == HAL_OK)
  {
	m.sub_id = CMD_ERASE_PAGES;
  }
  else
  {
	m.sub_id = ERASE_PAGES_ERROR;
  }

  usb_tx_message((uint8_t *)&m, sizeof(hid_raw_msg_type));  

}

void resp_dnload_msg(hid_raw_msg_type *raw_msg)
{
  hid_raw_msg_type *m = (hid_raw_msg_type *)raw_msg;
  m->msg_id = DNLOAD_MSG_ID;
}


uint8_t dnload_wr_buffer_handle(hid_raw_msg_type *raw_msg)
{
  hid_raw_msg_type m;
  uint8_t next_tr = 0;

  memset(&m, 0x0, sizeof(hid_raw_msg_type));
  m.msg_id = DNLOAD_MSG_ID;

  do
  {
    if((DLOAD_BUFF_SIZE - rcv_packet.offset) < raw_msg->data_len)
    {
      m.sub_id = WRITE_DATA_ERROR;
	  break;
    }
    
    memcpy(&rcv_packet.buff[rcv_packet.offset], raw_msg->data, raw_msg->data_len);
    rcv_packet.offset += raw_msg->data_len;
    if(rcv_packet.offset >= DLOAD_BUFF_SIZE)
    {
      next_tr = 1;
    }
    else
    {
      m.sub_id = CMD_WRITE_DATA; 
    }
	
  }while(0);

  if(!next_tr) usb_tx_message((uint8_t *)&m, sizeof(hid_raw_msg_type));

  return next_tr;
}

void dnload_write_data(uint8_t is_final)
{
  hid_raw_msg_type m;
  HAL_StatusTypeDef status;
  if(flash_wr_offset == 0)
  {
    memcpy(flash_1st_addr, rcv_packet.buff, 4);
    memset(rcv_packet.buff, 0x0, 4);
  }
  status = flash_dload_write(flash_wr_offset, rcv_packet.offset, rcv_packet.buff);
  flash_wr_offset += rcv_packet.offset;


  memset(&m, 0x0, sizeof(hid_raw_msg_type));
  m.msg_id = DNLOAD_MSG_ID;

  if(status == HAL_OK)
  {
    m.sub_id = is_final ? CMD_FINISH_DLOAD : CMD_WRITE_DATA;
  }
  else
  {
    m.sub_id = WRITE_DATA_ERROR;
  }

  usb_tx_message((uint8_t *)&m, sizeof(hid_raw_msg_type));

}

void dnload_write_final(hid_raw_msg_type *raw_msg)
{
  memcpy(&rcv_packet.buff[rcv_packet.offset], raw_msg->data, raw_msg->data_len);
  rcv_packet.offset += raw_msg->data_len;
}

void dnload_msg_dispatch(hid_raw_msg_type *raw_msg)
{
  if(raw_msg->msg_id == SET_DNLOAD_MODE)
  {
    if(raw_msg->sub_id == 1)
    {
      start_resp_msg(1);
      dnload_state = DNLOAD_CMD_WAIT_S;
    }
#ifdef D_DWONLOAD_PROG_STR
    else
    {
      screen_disp_clear();
      screen_disp_bitmap(0, 0, 9, 7, (uint8_t*)confirm_check_icon);
      screen_disp_bitmap(114, 2, 7, 7, (uint8_t*)x_button_icon);
      screen_disp_str(8,22, "F/W Upgrade OK ?", &Font_7x10);
      
      dnload_state = DNLOAD_READY_S;
    }
#endif/* D_DWONLOAD_PROG_STR */
  }
  else if(raw_msg->msg_id == DNLOAD_MSG_ID)
  {
    switch(raw_msg->sub_id)
    {
      case CMD_ERASE_PAGES:
//#ifdef D_DWONLOAD_PROG_STR	  	
        screen_disp_clear(); 
        screen_disp_str(2,15, "Update in progress", &Font_7x10);
//#endif		
	  	dnload_state = DNLOAD_ERASE_PAGE_S;
		
		FLASH_UNLOCK(); /* Disabled Flash Lock */
        break;
      case CMD_WRITE_DATA:
        if(dnload_wr_buffer_handle(raw_msg))
        {
          dnload_state = DNLOAD_WR_PROG_S;
        }
        break;
      case CMD_FINISH_DLOAD:
        dnload_write_final(raw_msg);
        dnload_state = DNLOAD_FINISH_S;
        break;
      default:
        break;
    }
  }
}

void flash_dnload_state(uint32_t *evt_mask, hid_raw_msg_type *raw_msg)
{  
  switch(dnload_state)
  {
    case DNLOAD_NONE_S:
      break;
	  
    case DNLOAD_READY_S:
      if((*evt_mask & BUTTON_1_EVENT) == BUTTON_1_EVENT)
      {
        *evt_mask &= ~BUTTON_1_EVENT;
		start_resp_msg(1);
		dnload_state = DNLOAD_CMD_WAIT_S;
      }
      if((*evt_mask & BUTTON_2_EVENT) == BUTTON_2_EVENT)
      {
        *evt_mask &= ~BUTTON_2_EVENT;
		start_resp_msg(0);
		dnload_state = DNLOAD_CANCEL_S;
#ifdef D_DWONLOAD_PROG_STR		
        screen_disp_clear();
        screen_disp_str(2,15, "    User Canceled", &Font_7x10);
#endif		
      }
      break;
	  
    case DNLOAD_ERASE_PAGE_S:
	  dnload_erase_page(raw_msg);
	  dnload_state = DNLOAD_CMD_WAIT_S;
	  rcv_packet.offset = 0;
	  flash_wr_offset = 0;
      break;
	  
    case DNLOAD_WR_PROG_S:
      dnload_write_data(0);
      memset(&rcv_packet, 0x0, sizeof(dload_packet_type));
	  dnload_state = DNLOAD_CMD_WAIT_S;
      break;
	  
    case DNLOAD_CMD_WAIT_S:
      break;
	  
    case DNLOAD_FINISH_S:
      dnload_write_data(1);
	  (void)flash_dload_write(0, 4, flash_1st_addr);
	  memset(&rcv_packet, 0x0, sizeof(dload_packet_type));
	  dnload_state = DNLOAD_COMPLETE_S;
	  
	  FLASH_LOCK(); /* Enabled Flash Lock */

//#ifdef D_DWONLOAD_PROG_STR
      screen_disp_clear();
      screen_disp_str(2,15, "    Complete !!", &Font_7x10);
//#endif	  
      break;
	  
    case DNLOAD_CANCEL_S:
      break;
    case DNLOAD_COMPLETE_S:
      break;
	  
    default:
      break;
  
  }
}

static flash_fd_type flash_fd;

void flash_erase_fd_page(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PAGEError;

  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FD_START_ADDR;
  EraseInitStruct.NbPages     = 1;

  if(HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
  {
    // error..
  }
}

void flash_write_fd(void)
{
  uint8_t i;
  
  FLASH_UNLOCK();
  
  flash_erase_fd_page();

  for(i=0; i<FLASH_PIN_MAX;i++)
  {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_PIN_CACHED_ADDR+4*i, flash_fd.pin_cached[i]);
  }
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_PIN_LEN_ADDR, flash_fd.pin_length);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_SCREEN_LOCK_ADDR, flash_fd.scr_lock);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_LANGUAGE_ADDR, flash_fd.language);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_DISP_INV_ADDR, flash_fd.disp_inverse);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_DISP_BR_ADDR, flash_fd.disp_brightness);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_HAS_SEED_ADDR, flash_fd.has_seed);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FD_BOOT_HW_TEST_ADDR, flash_fd.boot_hw_test);

  FLASH_LOCK();
}

void flash_fd_init(void)
{
  FLASH_UNLOCK();
  flash_erase_fd_page();
  FLASH_LOCK();
}

void flash_fd_read(device_feature_type *device_feature)
{
  uint8_t i;
  device_feature_type *df = (device_feature_type *)device_feature;
  
  memset(&flash_fd, 0x0, sizeof(flash_fd_type));

  for(i=0; i<FLASH_PIN_MAX;i++)
  {
    flash_fd.pin_cached[i] = *(__IO uint32_t *)(FD_PIN_CACHED_ADDR+4*i);
  }
  flash_fd.pin_length = *(__IO uint32_t *)FD_PIN_LEN_ADDR;
  flash_fd.scr_lock = *(__IO uint32_t *)FD_SCREEN_LOCK_ADDR;
  flash_fd.language = *(__IO uint32_t *)FD_LANGUAGE_ADDR;
  flash_fd.disp_inverse = *(__IO uint32_t *)FD_DISP_INV_ADDR;
  flash_fd.disp_brightness = *(__IO uint32_t *)FD_DISP_BR_ADDR;
  flash_fd.has_seed = *(__IO uint32_t *)FD_HAS_SEED_ADDR;
  flash_fd.boot_hw_test = *(__IO uint32_t *)FD_BOOT_HW_TEST_ADDR;

  df->pin_status.pin_len = (uint8_t)flash_fd.pin_length == 0xff ? 0 : (uint8_t)flash_fd.pin_length;
  for(i=0;i<FLASH_PIN_MAX && i<df->pin_status.pin_len;i++)
  {
    df->pin_status.pin_code[i] = (uint8_t)flash_fd.pin_cached[i];
  }
  df->language = (uint8_t)flash_fd.language == 0xff ? 0 : (uint8_t)flash_fd.language;
  df->scr_lock = (uint8_t)flash_fd.scr_lock == 0 ? 10 : (uint8_t)flash_fd.scr_lock;
  df->disp_bright = (uint8_t)flash_fd.disp_brightness == 0 ? 5 : (uint8_t)flash_fd.disp_brightness;
  df->has_seed = (uint8_t)flash_fd.has_seed == 0xff ? 0 : (uint8_t)flash_fd.has_seed;
  df->boot_hw_test = (uint8_t)flash_fd.boot_hw_test;
}

void flash_fd_pin_cache(uint8_t* val, uint8_t len)
{
  uint8_t i;
  
  for(i=0; i<FLASH_PIN_MAX && i<len; i++)
  {
    flash_fd.pin_cached[i] = (uint32_t)val[i];
  }
  flash_fd.pin_length = (uint32_t)len;
  flash_write_fd();
}

void flash_fd_scr_lock(uint8_t val)
{
  flash_fd.scr_lock = (uint32_t)val;
  flash_write_fd();
}

void flash_fd_language(uint8_t val)
{
  flash_fd.language = (uint32_t)val;
  flash_write_fd();
}

void flash_fd_disp_inverse(uint8_t val)
{
  flash_fd.disp_inverse = (uint32_t)val;
  flash_write_fd();
}

void flash_fd_disp_bright(uint8_t val)
{
  flash_fd.disp_brightness = (uint32_t)val;
  flash_write_fd();
}

void flash_fd_has_seed(uint8_t val)
{
  flash_fd.has_seed = (uint32_t)val;
  flash_write_fd();
}

void flash_fd_boot_hw_test(uint8_t val)
{
  flash_fd.boot_hw_test = (uint32_t)val;
  flash_write_fd();
}