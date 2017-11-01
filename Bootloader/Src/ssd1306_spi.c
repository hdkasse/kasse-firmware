/**
  ******************************************************************************
  * @file    ssd1360_spi.c
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

#include "ssd1306_spi.h"
#include "spi.h"
#include "stm32l0xx_hal.h"
#include <stdio.h>
#include <string.h>
/*================ Define ====================*/

#define SSD_USE_SPI
#ifndef SSD_USE_SPI
#include "i2c.h"
//#define SSD_USE_I2C
#endif

//#define BD_WS10
#define BD_WS20

#ifdef BD_WS10
#define NORMAL_ANGLE_DISPLAY
#else
//#define NORMAL_ANGLE_DISPLAY
#endif

#define SPI_CS_LOW()  HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, GPIO_PIN_RESET)
#define SPI_CS_HIGH()  HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, GPIO_PIN_SET)

#define SPI_DC_LOW()  HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin, GPIO_PIN_RESET)
#define SPI_DC_HIGH()  HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin, GPIO_PIN_SET)
#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

/*================ Variable ====================*/

typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} OLED1306_t;

static OLED1306_t OLED1306;

#define OLED_BUF_MAX (OLED1306_WIDTH * OLED1306_HEIGHT)/8

static uint8_t  OLED1306_Buffer[OLED_BUF_MAX];
static uint8_t  save_screen_buffer[OLED_BUF_MAX];
/*================ FUNCTION ====================*/
void OLED_DELAY(uint32_t delay)
{
  HAL_Delay(delay);
}


static void OLED_SPI_ERROR(void)
{
  HAL_SPI_DeInit(&hspi1);
  MX_SPI1_Init();
}

void OLED_write_command(uint8_t value)
{
  HAL_StatusTypeDef status = HAL_OK;
#ifdef SSD_USE_SPI
  SPI_CS_LOW();
  SPI_DC_LOW();

  status = HAL_SPI_Transmit(&hspi1, (uint8_t*)&value, 1,  10);
  if(status != HAL_OK)
  {
    OLED_SPI_ERROR();
  }
  else
  {
    //OK
  }

  SPI_CS_HIGH();
#else
  uint8_t pData[2];
  pData[0] = 0x00;
  pData[1] = value;
  status = HAL_I2C_Master_Transmit(&hi2c1,0x78,pData,2,10);
  //HAL_I2C_Mem_Write(&hi2c1,0x78,0x00,1,&value,1,10);
#endif
}

void OLED_write_data(uint8_t *pData, uint32_t size)
{
  HAL_StatusTypeDef status = HAL_OK;
#ifdef SSD_USE_SPI
  SPI_CS_LOW();
  //OLED_DELAY(1);
  SPI_DC_HIGH();

  status = HAL_SPI_Transmit(&hspi1, pData, size,  10);
  if(status != HAL_OK)
  {
    OLED_SPI_ERROR();
  }
  else
  {
    //OK
  }

  OLED_DELAY(1);

  SPI_CS_HIGH();
#else
  uint8_t i;
  uint8_t data[(OLED1306_WIDTH * OLED1306_HEIGHT)/8+1];
  
  data[0] = 0x40;
  for(i=0;i<size;i++)
    data[i+1]= pData[i];
  status = HAL_I2C_Master_Transmit(&hi2c1,0x78,data,size+1,50);
  //status = HAL_I2C_Mem_Write(&hi2c1,0x78,0x40,1,pData,size,50);
#endif
}


void OLED_init(void)
{
  OLED_set_display_on_off(DISPLAY_OFF);
  OLED_set_addressing_mode(PAGE_ADDRESSING_MODE); // Must set First!!!
  //OLED_set_display_clock(0x81);
  OLED_set_display_clock(0xF0);
  OLED_set_multiplex_ratio(0x3F); //display Y start point (1F)
  //change for LCD Dot(.) not clear..
#ifdef NORMAL_ANGLE_DISPLAY
  OLED_set_start_line(0x00); 
#else
  OLED_set_start_line(0x01); 
#endif
  OLED_set_charge_pump(ENABLE_CHARGE_PUMP);
  OLED_set_start_page(0); 
#ifdef NORMAL_ANGLE_DISPLAY
  OLED_set_segment_remap(COLUMN_ADDRESS_0_MAPPED_TO_SEG0);
  OLED_set_common_output_scan_direction(SCAN_FROM_COM0_TO_63); 
#else //screen angle reverse
  OLED_set_segment_remap(COLUMN_ADDRESS_0_MAPPED_TO_SEG127); 
  OLED_set_common_output_scan_direction(SCAN_FROM_COM63_TO_0); 
#endif
  OLED_set_start_column(0);

#ifdef COM_PIN_HW_CONFIG_128_32
#ifdef NORMAL_ANGLE_DISPLAY
  OLED_set_display_offset(0x00);
#else
  OLED_set_display_offset(0x1F);
#endif
  OLED_set_common_config(0x00);
#else //128_64
  OLED_set_display_offset(0x00);
  OLED_set_common_config(0x10);
#endif

  OLED_set_contrast(0xCF);
  //OLED_set_contrast(0xFF);
  OLED_set_precharge_period(0xF1);
  //OLED_set_precharge_period(0x22);
  //OLED_set_VCOMH(0x40);
  OLED_set_VCOMH(0x20);
  OLED_set_entire_display_on(NORMAL_DISPLAY);
  OLED_set_normal_or_inverse_display(NON_INVERTED_DISPLAY);
//  OLED_set_display_on_off(DISPLAY_ON);

//OLED_set_column_address(0,0x7F); //default value
//OLED_set_page_address(0,0x7); //default value
  OLED_fill(BLACK);
  //OLED_gotoxy(0,0);
  //OLED_set_display_on_off(DISPLAY_ON); //19-19
  OLED_screen_update();

  OLED1306.CurrentX = 0;
  OLED1306.CurrentY = 0;
  
  /* Initialized OK */
  OLED1306.Initialized = 1;
}

void OLED_gotoxy(uint8_t x, uint8_t y)
{
  OLED_write_command(SET_PAGE_START_ADDRESS_CMD + y);
  OLED_write_command((x & 0x0F) | SET_LOWER_COLUMN_START_ADDRESS_CMD);
  OLED_write_command(((x & 0xF0) >> 0x04) | SET_HIGHER_COLUMN_START_ADDRESS_CMD);
}


void OLED_SetCursor(uint8_t x, uint8_t y) 
{
  /* Set write pointers */
  OLED1306.CurrentX = x;
  OLED1306.CurrentY = y;
}

void OLED_fill(OLED_COLOR color)
{
  uint32_t i;
  
  for(i = 0; i < OLED_BUF_MAX*sizeof(uint8_t); i++)
  {
    OLED1306_Buffer[i] = (color == BLACK) ? 0x00 : 0xFF;
  }
  //OLED_screen_update();
}

void OLED_screen_page_update2(OLED_page_type page)
{
  OLED_write_command(SET_PAGE_START_ADDRESS_CMD + page);
  OLED_write_command(SET_LOWER_COLUMN_START_ADDRESS_CMD);
  OLED_write_command(SET_HIGHER_COLUMN_START_ADDRESS_CMD);    
  OLED_write_data(&OLED1306_Buffer[OLED1306_WIDTH*page], OLED1306_WIDTH);
  //OLED_set_display_on_off(DISPLAY_ON);
}

void OLED_screen_page_update(OLED_page_type page)
{
  OLED_write_command(SET_PAGE_START_ADDRESS_CMD + page);
  OLED_write_command(SET_LOWER_COLUMN_START_ADDRESS_CMD);
  OLED_write_command(SET_HIGHER_COLUMN_START_ADDRESS_CMD);
  for(uint8_t i=8; i<OLED1306_WIDTH; i+=8)
  {
    OLED_write_data(&OLED1306_Buffer[i*page], i);
    OLED_DELAY(50);
  }
  //OLED_set_display_on_off(DISPLAY_ON);
}

void OLED_screen_update(void)
{
  uint8_t page;//, x_pos;

  for(page = PAGE0; page <= PAGE_END; page++)
  {
    OLED_write_command(SET_PAGE_START_ADDRESS_CMD + page); //OLED_set_start_page(page);//
    OLED_write_command(SET_LOWER_COLUMN_START_ADDRESS_CMD);
    OLED_write_command(SET_HIGHER_COLUMN_START_ADDRESS_CMD);
    
    //for(x_pos = 0; x_pos <= 127; x_pos++)
    {
      OLED_write_data(&OLED1306_Buffer[OLED1306_WIDTH*page], OLED1306_WIDTH);
    }
  }
  OLED_set_display_on_off(DISPLAY_ON);
}


void OLED_save_screen(void)
{
  memcpy(save_screen_buffer, OLED1306_Buffer, (sizeof(OLED1306_Buffer)/sizeof(OLED1306_Buffer[0])));
}

void OLED_save_screen_update(void)
{
  uint8_t page;//, x_pos;

  for(page = PAGE0; page <= PAGE_END; page++)
  {
    OLED_write_command(SET_PAGE_START_ADDRESS_CMD + page); //OLED_set_start_page(page);//
    OLED_write_command(SET_LOWER_COLUMN_START_ADDRESS_CMD);
    OLED_write_command(SET_HIGHER_COLUMN_START_ADDRESS_CMD);
    
    //for(x_pos = 0; x_pos <= 127; x_pos++)
    {
      memcpy(OLED1306_Buffer, save_screen_buffer , (sizeof(OLED1306_Buffer)/sizeof(OLED1306_Buffer[0])));
      OLED_write_data(&save_screen_buffer[OLED1306_WIDTH*page], OLED1306_WIDTH);
    }
  }
  OLED_set_display_on_off(DISPLAY_ON);
}

void OLED_clear_screen(void)
{
  OLED_fill(BLACK);
}

void OLED_set_start_column(uint8_t n)
{
  OLED_write_command(SET_LOWER_COLUMN_START_ADDRESS_CMD + n);//(n % 16));
  OLED_write_command(SET_HIGHER_COLUMN_START_ADDRESS_CMD + n);//(n / 16));
}

void OLED_set_addressing_mode(uint8_t n)
{
  OLED_write_command(SET_MEMORY_ADDRESSING_MODE_CMD);
  OLED_write_command(n);
}

void OLED_set_column_address(uint8_t r, uint8_t t)
{
  OLED_write_command(SET_COLUMN_ADDRESS_CMD);
  OLED_write_command(r);
  OLED_write_command(t);
}

void OLED_set_page_address(uint8_t r, uint8_t t)
{
  OLED_write_command(SET_PAGE_ADDRESS_CMD);
  OLED_write_command(r);
  OLED_write_command(t);
}

void OLED_set_start_line(uint8_t n)
{
  OLED_write_command(SET_DISPLAY_START_LINE_CMD | n);
}

void OLED_set_contrast(uint8_t n)
{
  OLED_write_command(SET_CONTRAST_CONTROL_CMD);
  OLED_write_command(n);
}

void OLED_set_charge_pump(uint8_t n)
{
  OLED_write_command(SET_CHARGE_PUMP_CMD);
  OLED_write_command(SET_HIGHER_COLUMN_START_ADDRESS_CMD | n);
}

void OLED_set_segment_remap(uint8_t n)
{
  OLED_write_command(SET_SEGMENT_REMAP_CMD | n);
}

void OLED_set_entire_display_on(uint8_t n)
{
  OLED_write_command(SET_ENTIRE_DISPLAY_ON_CMD | n);
}

void OLED_set_normal_or_inverse_display(uint8_t n)
{
  OLED_write_command(SET_NORMAL_OR_INVERSE_DISPLAY_CMD | n);
}

void OLED_set_multiplex_ratio(uint8_t n)
{
  OLED_write_command(SET_MULTIPLEX_RATIO_CMD);
  OLED_write_command(n);
}

void OLED_set_display_on_off(uint8_t n)
{
  OLED_write_command(SET_DISPLAY_ON_OR_OFF_CMD + n);
}

void OLED_set_start_page(uint8_t n)
{
  OLED_write_command(SET_PAGE_START_ADDRESS_CMD | n);
}

void OLED_set_common_output_scan_direction(uint8_t n)
{
  OLED_write_command(SET_COM_OUTPUT_SCAN_DIRECTION_CMD | n);
}


void OLED_set_display_offset(uint8_t n)
{
  OLED_write_command(SET_DISPLAY_OFFSET_CMD);
  OLED_write_command(n);
}


void OLED_set_display_clock(uint8_t n)
{
  OLED_write_command(SET_DISPLAY_CLOCK_CMD);
  OLED_write_command(n);
}


void OLED_set_precharge_period(uint8_t n)
{
  OLED_write_command(SET_PRE_CHARGE_PERIOD_CMD);
  OLED_write_command(n);
}


void OLED_set_common_config(uint8_t n)
{
  OLED_write_command(SET_COMMON_HW_CONFIG_CMD);
  OLED_write_command(0x02 | n);
}


void OLED_set_VCOMH(uint8_t n)
{
  OLED_write_command(SET_VCOMH_LEVEL_CMD);
  OLED_write_command(n);
}


void OLED_set_NOP(void)
{
  OLED_write_command(SET_NOP_CMD);
}


void OLED_cursor(uint8_t x_pos, uint8_t y_pos)
{
  uint8_t i;
  
  if(y_pos != 0)
  {
    if(x_pos == 1)
    {
      OLED_gotoxy(0x00, (y_pos + 0x02));
    }
    else
    {
      OLED_gotoxy((0x50 + ((x_pos - 0x02) * 0x06)), (y_pos + 0x02));
    }
  
    for(i = 0; i < 6; i++)
    {
      //OLED_write_data(0xFF, 1);
    }
  }
}


void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color)
{
	if (x >= OLED1306_WIDTH || y >= OLED1306_HEIGHT) 
	{
		return;
	}

	/* Check if pixels are inverted */
	if (OLED1306.Inverted) 
	{
		color = (OLED_COLOR)!color;
	}

   /* Set color */
	if (color == WHITE)
	{
		OLED1306_Buffer[x + (y / 8) * OLED1306_WIDTH] |= 1 << (y % 8);
	} 
	else 
	{
		OLED1306_Buffer[x + (y / 8) * OLED1306_WIDTH] &= ~(1 << (y % 8));
	}
}

void OLED_drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h) 
{
  uint16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;
  int16_t j=0;
  int16_t i=0;

  for(j=0; j<h; j++, y++) {
    for(i=0; i<w; i++ ) {
      if(i & 7){
	    byte <<= 1;
      }
      else{
	    byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      }
      OLED_DrawPixel(x+i, y, (byte & 0x80) ? WHITE : BLACK);
    }
  }

}

void OLED_set_horizon_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x)
{
  OLED_write_command(SET_RIGHT_HORIZONTAL_SCROLL);
  OLED_write_command(0X00); //Dummy
  OLED_write_command(start_page); //start page 0~7
  OLED_write_command(0X00); //interval
  OLED_write_command(end_page); //end page 0~7
  OLED_write_command(scroll_start_x); //start column
  OLED_write_command(scroll_end_x); //end column
  OLED_write_command(SET_ACTIVATE_SCROLL);
}

void OLED_set_horizon_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x)
{
  OLED_write_command(SET_LEFT_HORIZONTAL_SCROLL);
  OLED_write_command(0X00);
  OLED_write_command(start_page);
  OLED_write_command(0X00);
  OLED_write_command(end_page);
  OLED_write_command(scroll_start_x);
  OLED_write_command(scroll_end_x);
  OLED_write_command(SET_ACTIVATE_SCROLL);
}
#if 1
void OLED_set_vertical_scroll_right(uint8_t start_page, uint8_t end_page)
{
  OLED_write_command(SET_VERTICAL_SCROLL_AREA);
  OLED_write_command(0X00);
  OLED_write_command(OLED1306_HEIGHT);
  OLED_write_command(SET_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  OLED_write_command(0X00);
  OLED_write_command(start_page);
  OLED_write_command(0X00);
  OLED_write_command(end_page);
  OLED_write_command(0X01);
  OLED_write_command(SET_ACTIVATE_SCROLL);
}

void OLED_set_vertical_scroll_left(uint8_t start_page, uint8_t end_page)
{
  OLED_write_command(SET_VERTICAL_SCROLL_AREA);
  OLED_write_command(0X00);
  OLED_write_command(OLED1306_WIDTH);
  OLED_write_command(SET_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  OLED_write_command(0X00);
  OLED_write_command(start_page);
  OLED_write_command(0X00);
  OLED_write_command(end_page);
  OLED_write_command(0X01);
  OLED_write_command(SET_ACTIVATE_SCROLL);
}
#endif
void OLED_start_scroll(void)
{
  OLED_write_command(SET_ACTIVATE_SCROLL);
}

void OLED_stop_scroll(void)
{
  OLED_write_command(SET_DEACTIVATE_SCROLL);
}

void OLED_set_content_right_scroll(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x)
{
  OLED_write_command(SET_RIGHT_CONTENT_SCROLL);
  OLED_write_command(0X00); //Dummy
  OLED_write_command(start_page); //start page 0~7
  OLED_write_command(0X01); //dummy
  OLED_write_command(end_page); //end page 0~7
  OLED_write_command(scroll_start_x); //start column
  OLED_write_command(scroll_end_x); //end column
}

void OLED_set_content_left_scroll(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x)
{
  OLED_write_command(SET_LEFT_CONTENT_SCROLL);
  OLED_write_command(0X00);
  OLED_write_command(start_page);
  OLED_write_command(0X01);
  OLED_write_command(end_page);
  OLED_write_command(scroll_start_x);
  OLED_write_command(scroll_end_x);
}

//Not complete operating.. once after decrease, cannot recover contrast level
void OLED_set_blink_fadeout(uint8_t mode)
{
   OLED_write_command(SET_BLINK_FADEOUT_CMD);
   OLED_write_command(mode|0x00);
}

void OLED_change_disp_direction(void)
{
#ifdef NORMAL_ANGLE_DISPLAY
  static uint8_t invert=0;
#else
  static uint8_t invert=1;
#endif
  uint8_t disp_offset[] ={0x00, 0x1F};
  uint8_t remap, scan;

  invert ^= 1;
  remap = invert;
  scan = (invert <<3);

  if(invert) OLED_set_start_line(0x01);
  else OLED_set_start_line(0x00);

  OLED_set_display_offset(disp_offset[invert]);
  OLED_set_segment_remap(remap);
  OLED_set_common_output_scan_direction(scan);
  OLED_screen_update();
}

void OLED_change_contrast(uint8_t val)
{
  if(val < 0x01) val = 0x01;
  if(val > 0xFF) val = 0xFF;

  OLED_set_contrast(val);
}

