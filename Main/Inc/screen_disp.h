/**
  ******************************************************************************
  * @file    screen_disp.h
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

#ifndef __SCREEN_DISP_
#define __SCREEN_DISP_

#include "ssd1306_spi.h"


typedef struct {
  const uint8_t width;    /*!< Font width in pixels */
  const uint8_t height;   /*!< Font height in pixels */
  const uint16_t *data; /*!< Pointer to data font data array */
} font_data_type;

extern font_data_type Font_7x10;
extern font_data_type Font_11x18;

void screen_disp_init(void);
void screen_clear(OLED_COLOR color);
void screen_disp_clear(void);


void screen_disp_bitmap
(
  uint8_t     xtop, 
  uint8_t     ytop, 
  uint8_t     width, 
  uint8_t     height, 
  uint8_t     *bmp_data
);

char screen_disp_char
(
  uint16_t          xpos,
  uint16_t          ypos,
  char              ch, 
  font_data_type    *cfont,
  uint8_t           screen_update
);

int screen_disp_str
(
  uint16_t          xpos,
  uint16_t          ypos,
  char*             str, 
  font_data_type    *cfont
);

char screen_disp_num
(
  uint16_t          xpos,
  uint16_t          ypos,
  uint16_t              num, 
  font_data_type    *cfont
);

void screen_disp_line
(
  uint16_t     x1, 
  uint16_t     y1, 
  uint16_t     x2, 
  uint16_t     y2
);

void screen_disp_rectangle
(
  uint16_t     x1,
  uint16_t     y1, 
  uint16_t     x2,
  uint16_t     y2
);

void screen_disp_circle
(
  uint16_t     x1,
  uint16_t     y1, 
  uint16_t     r
);

void screen_disp_sine
(
  uint16_t     peak, 
  uint8_t      mode
);

void screen_disp_stop_scroll(void);
void screen_disp_horizon_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round);
void screen_disp_horizon_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round);
void screen_disp_content_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round);
void screen_disp_content_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round);
void screen_disp_vertical_right_left(uint8_t direct, uint8_t start, uint8_t end);
void screen_disp_verify_all_page(void);
void screen_disp_verify_page(uint8_t start, uint8_t end);
void screen_disp_change_direction(void);
void screen_disp_change_contrast(uint8_t val);
void screen_disp_backup_screen(void);
void screen_disp_restore_screen(void);


#define UNDER_LINE_WIDTH    8
#define SEPARATOR_WIDTH     4
#define CODE_NUMBER         10
#define CODE_START_X        3
#define CODE_START_Y        30

void screen_disp_code_init(void);

void screen_disp_code_num(uint8_t order, uint8_t number);
void screen_disp_code_char(uint8_t order, char ch);
void screen_disp_underline(uint8_t order, uint8_t enabled);


void screen_disp_brightness(uint8_t level);
void screen_disp_inverse(uint8_t enabled);


#endif/* __SCREEN_DISP_ */
