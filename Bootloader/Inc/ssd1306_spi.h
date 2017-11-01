/**
  ******************************************************************************
  * @file    ssd1360_spi.h
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
//#include "spi.h"
#include "gpio.h"

#ifndef SSD1306_SPI_USED
#define SSD1306_SPI_USED

#define COM_PIN_HW_CONFIG_128_32

#define ABS(x)   ((x) > 0 ? (x) : -(x))

//SRAM Display buffer 128x64
#define OLED1306_WIDTH           (128)
#ifdef COM_PIN_HW_CONFIG_128_32
#define OLED1306_HEIGHT          32
#else
#define OLED1306_HEIGHT          64
#endif

#define SCROLL_X_FIRST 0x0
#define SCROLL_X_END 0x7F

#define OLED_1ST_X  2
#define OLED_1S_Y 0

#define DATA 1 //DC High
#define CMD 0 //DC Low

//1. Fundamental CMD
#define SET_CONTRAST_CONTROL_CMD 0x81
#define SET_ENTIRE_DISPLAY_ON_CMD 0xA4
#define SET_NORMAL_OR_INVERSE_DISPLAY_CMD 0xA6
#define SET_DISPLAY_ON_OR_OFF_CMD 0xAE

//2. Scrolling CMD
#define SET_RIGHT_HORIZONTAL_SCROLL 0x26
#define SET_LEFT_HORIZONTAL_SCROLL 0x27
#define SET_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SET_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A
#define SET_DEACTIVATE_SCROLL 0x2E
#define SET_ACTIVATE_SCROLL 0x2F
#define SET_VERTICAL_SCROLL_AREA 0xA3
#define SET_RIGHT_CONTENT_SCROLL 0x2C
#define SET_LEFT_CONTENT_SCROLL 0x2D

//3. Addressing setting CMD
#define SET_LOWER_COLUMN_START_ADDRESS_CMD 0x00
#define SET_HIGHER_COLUMN_START_ADDRESS_CMD 0x10
#define SET_MEMORY_ADDRESSING_MODE_CMD 0x20
#define SET_COLUMN_ADDRESS_CMD 0x21
#define SET_PAGE_ADDRESS_CMD 0x22
#define SET_PAGE_START_ADDRESS_CMD 0xB0

//4. HW configuration CMD
#define SET_DISPLAY_START_LINE_CMD 0x40
#define SET_SEGMENT_REMAP_CMD 0xA0
#define SET_MULTIPLEX_RATIO_CMD 0xA8
#define SET_COM_OUTPUT_SCAN_DIRECTION_CMD 0xC0
#define SET_DISPLAY_OFFSET_CMD 0xD3
#define SET_COMMON_HW_CONFIG_CMD 0xDA

//5. Timing, Driving Scheme CMD
#define SET_DISPLAY_CLOCK_CMD 0xD5
#define SET_PRE_CHARGE_PERIOD_CMD 0xD9
#define SET_VCOMH_LEVEL_CMD 0xDB
#define SET_NOP_CMD 0xE3

//6.Advaned graphic CMD
#define SET_BLINK_FADEOUT_CMD 0x23

//charge pump CMD
#define SET_CHARGE_PUMP_CMD 0x8D


#define HORIZONTAL_ADDRESSING_MODE 0x00
#define VERTICAL_ADDRESSING_MODE 0x01
#define PAGE_ADDRESSING_MODE 0x02

#define DISABLE_CHARGE_PUMP 0x00
#define ENABLE_CHARGE_PUMP 0x04

#define COLUMN_ADDRESS_0_MAPPED_TO_SEG0 0x00
#define COLUMN_ADDRESS_0_MAPPED_TO_SEG127 0x01

#define NORMAL_DISPLAY 0x00
#define ENTIRE_DISPLAY_ON 0x01

#define NON_INVERTED_DISPLAY 0x00
#define INVERTED_DISPLAY 0x01

#define DISPLAY_OFF 0x00
#define DISPLAY_ON 0x01

#define SCAN_FROM_COM0_TO_63 0x00
#define SCAN_FROM_COM63_TO_0 0x08

#define DISABLE_BLINK_FADEOUT 0x00
#define ENABLE_FADEOUT 0x20
#define ENABLE_BLINK 0x30

typedef enum {
	BLACK = 0x00, /*!< Black color, no pixel */
	WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} OLED_COLOR;

typedef enum {
  PAGE0 = 0,
  PAGE1 = 1,
  PAGE2 = 2,
  PAGE3 = 3,
#ifdef COM_PIN_HW_CONFIG_128_32
  PAGE_END = PAGE3,
#else
  PAGE4 = 4,
  PAGE5 = 5,
  PAGE6 = 6,
  PAGE7 = 7,
  PAGE_END = PAGE7,
#endif
} OLED_page_type;

 void OLED_DELAY(uint32_t delay);
void OLED_write_command(uint8_t value);
void OLED_write_data(uint8_t *pData, uint32_t size);
void OLED_init(void);
void OLED_gotoxy(uint8_t x, uint8_t y);
void OLED_SetCursor(uint8_t x, uint8_t y);
void OLED_fill(OLED_COLOR color);
void OLED_screen_page_update(OLED_page_type page);
void OLED_screen_update(void);
void OLED_clear_screen(void);
void OLED_set_start_column(uint8_t n);
void OLED_set_addressing_mode(uint8_t n);
void OLED_set_column_address(uint8_t r, uint8_t t);
void OLED_set_page_address(uint8_t r, uint8_t t);
void OLED_set_start_line(uint8_t n);
void OLED_set_contrast(uint8_t n);
void OLED_set_charge_pump(uint8_t n);
void OLED_set_segment_remap(uint8_t n);
void OLED_set_entire_display_on(uint8_t n);
void OLED_set_normal_or_inverse_display(uint8_t n);
void OLED_set_multiplex_ratio(uint8_t n);
void OLED_set_display_on_off(uint8_t n);
void OLED_set_start_page(uint8_t n);
void OLED_set_common_output_scan_direction(uint8_t n);
void OLED_set_display_offset(uint8_t n);
void OLED_set_display_clock(uint8_t n);
void OLED_set_precharge_period(uint8_t n);
void OLED_set_common_config(uint8_t n);
void OLED_set_VCOMH(uint8_t n);
void OLED_set_NOP(void);
void OLED_cursor(uint8_t x_pos, uint8_t y_pos);
void OLED_drawBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h);
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color);
//char OLED_WriteInt(uint16_t num, uint8_t x, uint8_t y, FontDef Font, OLED_COLOR color);
void OLED_set_horizon_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x);
void OLED_set_horizon_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x);
#if 1
void OLED_set_vertical_scroll_right(uint8_t start_page, uint8_t end_page);
void OLED_set_vertical_scroll_left(uint8_t start_page, uint8_t end_page);
#endif
void OLED_start_scroll(void);
void OLED_stop_scroll(void);
void OLED_set_content_right_scroll(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x);
void OLED_set_content_left_scroll(uint8_t start_page, uint8_t end_page, uint8_t scroll_start_x, uint8_t scroll_end_x);
void OLED_set_blink_fadeout(uint8_t mode);
void OLED_change_disp_direction(void);
void OLED_change_contrast(uint8_t val);
void OLED_save_screen(void);
void OLED_save_screen_update(void);

#endif

