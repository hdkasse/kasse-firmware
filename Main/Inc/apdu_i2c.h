/**
  ******************************************************************************
  * @file    apdu_i2c.h
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

#include "stm32l0xx_hal.h"
#include "gpio.h"

#ifndef APDU_I2C
#define APDU_I2C

#define SECURE_I2C_ADDR (0x20<<1)

#define ABS(x)   ((x) > 0 ? (x) : -(x))

#define ARRY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define CLA_LEN 1
#define INS_LEN 1
#define P1_LEN 1
#define P2_LEN 1
#define Lc_LEN 1

#define MAX_I2C_DATA_LEN 254
#define HEDAER_LEN 5 //cla+ins+p1+p2+lc

#define APDU_CMD_DATA_LEN (MAX_I2C_DATA_LEN-HEDAER_LEN)
#define APDU_RES_DATA_LEN 70

typedef struct {
  uint8_t CLA; //instruction class
  uint8_t INS; //code
  uint8_t P1; // param1
  uint8_t P2; // param2
  uint8_t Lc; //Data length  
} APDU_cmd_header_type;

typedef struct{
  //uint8_t Lc; //Data length // move to header...
  uint8_t Data[APDU_CMD_DATA_LEN]; // Command parameters
} APDU_cmd_body_type;

typedef struct
{
  APDU_cmd_header_type header;
  APDU_cmd_body_type body;
} APDU_command_type;
#if 0
typedef struct
{
  APDU_cmd_header_type header;
  uint8_t Lc; //Data length
} APDU_command_no_data_type;
#endif
typedef struct {
  uint8_t Data[APDU_RES_DATA_LEN]; /* Data Body */
  //uint8_t SW1; // Trailer processing status
  //uint8_t SW2; //
} APDU_response_type;

typedef struct {
  uint8_t MSB;
  uint8_t LSB;
} APDU_resp_length_type;

typedef enum {
  APDU_PIN_VERIFY= 0x00,
  APDU_PIN_CHECK = 0x01,
  APDU_PIN_UPDATE = 0x02,
  APDU_PIN_RESET = 0x03,
  APDU_PIN_VALID = 0x04,
  APDU_PIN_MAX_NUM
} APDU_pincheck_type;

typedef enum {
  ISSUER_ID_NUM = 0,
  CARD_IMG_NUM = 1,
  CARD_DATA = 2,
  KEY_DERV_DATA = 3,
  SEC_CH_COUNTER = 4,
  KYE_INFO_TEMPL = 5,
  CARD_LIFE_CYCLE_DATA = 6,
  PLATF_ID = 7,
  PACK_ID_INFO = 8,
  HW_ID = 9,
  CARD_RSC_INFO = 10,
  EXT_CARD_RSC_INFO = 11,
  MAX_DATA_OBJECT
} get_data_object_type;

typedef struct {
  uint8_t p1;
  uint8_t p2;
  uint8_t len;
} get_data_cmd_p1p2_tag_type;

typedef struct {
  get_data_cmd_p1p2_tag_type tag_len;
  uint8_t mask_id[4];
  uint8_t rom_code_ver[4];
  uint8_t prd_config[4];
} tag_ff04_data_type;

typedef struct {
  get_data_cmd_p1p2_tag_type tag_len;
  uint8_t pack_ver[2];
} tag_ff05_data_type;

typedef struct {
  get_data_cmd_p1p2_tag_type tag_len;
  uint8_t master_prd_id[2]; //00DE
  uint8_t prd_id_num_comm_ch_prd[2];
  uint8_t mem_size; //03(320kbyte)
  uint8_t ips[4]; //7FBD6A20
  uint8_t in_prd_ver;
  uint8_t in_fw_revision[3];
  uint8_t custom_in_code[3];
  uint8_t neslib_ver[4]; //0104020A
  uint8_t RFU[8];
} tag_ff06_data_type;

typedef struct {
  get_data_cmd_p1p2_tag_type tag_len;
  uint8_t avail_memory[2]; //0xFFFF if not less than 64KB
  uint8_t num_of_app;
} tag_ff20_data_type;

typedef struct {
  get_data_cmd_p1p2_tag_type tag_len;
  uint8_t num_of_ap_tag; //0x81
  uint8_t len; //0x01
  uint8_t num_of_app;
  uint8_t free_nv_mem_tag; //0x82
  uint8_t len2; //0x04
  uint8_t free_nv_mem[4];
  uint8_t free_nv_tag; //0x83
  uint8_t len3; //0x02
  uint8_t free_nv[2];
} tag_ff21_data_type;

typedef struct {
  union {
    tag_ff04_data_type ff04;
    tag_ff05_data_type ff05;
    tag_ff06_data_type ff06;
    tag_ff20_data_type ff20;
    tag_ff21_data_type ff21;
  } get_data_tag_type;
  uint8_t sw1;
  uint8_t sw2;
} get_data_cmd_tag_data_type;

typedef struct {
  uint8_t key_divers_data[10];
  uint8_t key_info_data[2];
  uint8_t sequencd_cnt[2];
  uint8_t card_challenge[6];
  uint8_t card_crypto[8];
} init_update_resp_keyset_ver_type;

typedef struct {
  uint8_t len; //'10'
  uint8_t encrypt_key_val[16];
  uint8_t check_val_len; //'03'
  uint8_t check_val[3];
} put_des_key_data_field_type;

typedef struct {
  uint8_t new_key_set_ver;
  uint8_t key_idx1_algo_id; //'80'
  put_des_key_data_field_type key_idx1_key_ENC;
  uint8_t key_idx2_algo_id; //'80'
  put_des_key_data_field_type key_idx2_key_MAC;
  uint8_t key_idx3_algo_id; //'80'
  put_des_key_data_field_type key_idx3_key_KEK;
  uint8_t opt_end_of_key_set_id;  
} put_des_key_cmd_data_type;

typedef struct {
  uint8_t add_update_key_set_ver;
  uint8_t key_idx1_key_chk_val[3];
  uint8_t key_idx2_key_chk_val[3];
  uint8_t key_idx3_key_chk_val[3];
} put_des_key_resp_data_type;

typedef struct {
  uint8_t len; //0x10, 0x18, 0x20
  uint8_t encrypt_key_val[16];
  uint8_t check_val_len; //0x03
  uint8_t check_val[3];
} put_aes_key_data_field_type;

typedef struct {
  uint8_t new_key_set_ver;
  uint8_t key_idx1_algo_id; //0x88
  uint8_t key_idx1_len_of_key_data;
  put_aes_key_data_field_type key_idx1_ENC_key_data;
  uint8_t key_idx2_algo_id; //0x88
  uint8_t key_idx2_len_of_key_data;
  put_aes_key_data_field_type key_idx2_MAC_key_data;
  uint8_t key_idx3_algo_id; //0x88
  uint8_t key_idx3_len_of_key_data;
  put_aes_key_data_field_type key_idx3_KEK_key_data;
  uint8_t opt_end_of_key_set_id;  
} put_aes_key_cmd_data_type;

typedef struct {
  uint8_t add_update_key_set_ver;
  uint8_t key_idx1_key_chk_val[3];
  uint8_t key_idx2_key_chk_val[3];
  uint8_t key_idx3_key_chk_val[3];
} put_aes_key_resp_data_type;

// APDU reaceive callback type
typedef void (*apdu_rx_callback_t)(void * msg);
void APDU_received_from_hid(void *msg);

typedef void (*hid_apdu_callback_t)(void *msg);
void APDU_hid_send_apdu(void *msg);


void APDU_DELAY(uint32_t delay);
void APDU_reset(void);
HAL_StatusTypeDef APDU_write_cmd_len(uint8_t *pData, uint32_t size);
HAL_StatusTypeDef APDU_read_response_data(uint8_t *pData, uint8_t size);
HAL_StatusTypeDef APDU_read_resp_length_info(void);
HAL_StatusTypeDef APDU_get_timeout_data(uint8_t *rsp);
uint16_t ADPU_command_data_size(uint8_t lc_size);
HAL_StatusTypeDef APDU_commad_response(uint8_t *cmd, uint8_t *rsp);
HAL_StatusTypeDef APDU_commad_size_response(uint8_t *cmd, uint8_t size, uint8_t *rsp);
uint8_t APDU_HID_data_rx_process(uint8_t *in_data);
void APDU_HID_data_tx_process(uint8_t *tx_buf);
uint8_t APDU_data_process(uint8_t *in_data, uint8_t *out_data);
uint8_t APDU_command_test(void);
uint8_t APDU_set_init_seed(uint8_t *msg, uint8_t msg_len);
uint8_t APDU_set_factofy_init(void);
uint8_t APDU_set_pin_check(APDU_pincheck_type operate, uint8_t* data, uint8_t data_len, uint8_t *out_data);
uint8_t APDU_set_version(uint8_t *data);
#endif
