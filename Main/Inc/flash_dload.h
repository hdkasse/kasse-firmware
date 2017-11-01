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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_DLOAD_H
#define __FLASH_DLOAD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "msg_dispatch.h"
#include "user_type.h"


/* Base address of the Flash sectors */
#define FLASH_USER_START_ADDR (FLASH_BASE + 0x5000)
#define FLASH_USER_END_ADDR     (FLASH_BASE + 0x1FF80)   /* End @ of user Flash area */

/* Exported macro ------------------------------------------------------------*/
#define FLASH_UNLOCK() HAL_FLASH_Unlock()
#define FLASH_LOCK()   HAL_FLASH_Lock()

/* Exported types ------------------------------------------------------------*/
#define RCV_DATA_SIZE    56
#define DLOAD_BUFF_SIZE  (RCV_DATA_SIZE * 20)

typedef enum
{
  CMD_NONE                = 0x00,
  CMD_ERASE_PAGES         = 0x01,
  CMD_WRITE_DATA          = 0x02,
  CMD_FINISH_DLOAD        = 0x03,
  COMMAND_MAX
}dload_cmd_sub_id_type;

typedef struct
{
  uint16_t offset;
  uint8_t buff[DLOAD_BUFF_SIZE];
}dload_packet_type;

  
typedef enum
{
  DNLOAD_NONE_S       = 0,
  DNLOAD_READY_S,
  DNLOAD_START_S,
  DNLOAD_ERASE_PAGE_S,
  DNLOAD_WR_PROG_S,
  DNLOAD_FINISH_S,
  DNLOAD_CMD_WAIT_S,
  DNLOAD_CANCEL_S,
  DNLOAD_COMPLETE_S,
  DNLOAD_MAX_S
  
}dnload_state_type;

typedef struct
{
  uint32_t pin_cached;
  uint32_t scr_lock;
  uint32_t language;
  uint32_t disp_inverse;
  uint32_t disp_brightness;
  uint32_t has_seed;
  uint32_t assign_pphrase;
  uint32_t boot_hw_test;
}flash_fd_type;

#define FD_OFFSET 4
#define FD_START_ADDR        (FLASH_BASE + 0x1FC00)
#define FD_PIN_CACHED_ADDR   (FD_START_ADDR)
#define FD_SCREEN_LOCK_ADDR  (FD_PIN_CACHED_ADDR + FD_OFFSET)
#define FD_LANGUAGE_ADDR     (FD_SCREEN_LOCK_ADDR + FD_OFFSET)
#define FD_DISP_INV_ADDR     (FD_LANGUAGE_ADDR + FD_OFFSET)
#define FD_DISP_BR_ADDR      (FD_DISP_INV_ADDR + FD_OFFSET)
#define FD_HAS_SEED_ADDR     (FD_DISP_BR_ADDR + FD_OFFSET)
#define FD_ASSIGN_PPHRASE    (FD_HAS_SEED_ADDR + FD_OFFSET)
#define FD_BOOT_HW_TEST_ADDR (FD_ASSIGN_PPHRASE + FD_OFFSET)

#define SERIAL_NUM_ADDR      (FLASH_BASE + 0x1FE00)
#define SEC_EXTRA_VER        (FLASH_BASE + 0x1FE80)

/* Exported functions ------------------------------------------------------- */
HAL_StatusTypeDef flash_erase_all_page(uint32_t total_size);
HAL_StatusTypeDef flash_dload_write(uint32_t offset, uint32_t len, uint8_t *data);

void dnload_msg_dispatch(hid_raw_msg_type *raw_msg);
void flash_dnload_state(uint32_t *evt_mask, hid_raw_msg_type *raw_msg);

void flash_erase_fd_page(void);
void flash_fd_init(void);

void flash_write_fd(void);
void flash_fd_read(device_feature_type *device_feature);

void flash_fd_pin_permanent_fail(void);

void flash_fd_pin_cached(uint8_t val);

void flash_fd_scr_lock(uint8_t val);

void flash_fd_language(uint8_t val);

void flash_fd_disp_inverse(uint8_t val);

void flash_fd_disp_bright(uint8_t val);

void flash_fd_has_seed(uint8_t val);

void flash_fd_assign_pphrase(uint8_t val);

void flash_fd_boot_hw_test(uint8_t val);

uint32_t flash_sn_read(void);

void flash_sn_write(uint32_t val);

uint32_t flash_get_sec_extra_ver(void);

void flash_set_sec_extra_ver(uint32_t val);

#endif /* __FLASH_DLOAD_H */

