/**
  ******************************************************************************
  * @file    screen_disp.c
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


#include <math.h>

#include "ssd1306_spi.h"
#include "screen_disp.h"
#include "fonts.h"

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef ABS
#define ABS(x) ((x)>0?(x):-(x))
#endif

font_data_type Font_7x10  =  {7,10, Font7x10};
font_data_type Font_11x18 = {11,18, Font11x18};

static uint16_t currentX;
static uint16_t currentY;
static uint16_t screen_x_max;
static uint16_t screen_y_max;

void screen_disp_init(void)
{
  OLED_init();
  currentX = 0;
  currentY = 0;
  screen_x_max = OLED1306_WIDTH;
  screen_y_max = OLED1306_HEIGHT;
}

void screen_clear(OLED_COLOR color)
{
  OLED_fill(color);
  OLED_screen_update();
}

void screen_disp_clear(void)
{
  screen_clear(BLACK);
}

void screen_disp_bitmap
(
  uint8_t     xtop, 
  uint8_t     ytop, 
  uint8_t     width, 
  uint8_t     height, 
  uint8_t     *bmp_data
)
{
  OLED_drawBitmap(xtop, ytop,bmp_data, width, height);
  OLED_screen_update();
}

char screen_disp_char
(
  uint16_t          xpos,
  uint16_t          ypos,
  char              ch, 
  font_data_type    *cfont,
  uint8_t           screen_update
)
{
  uint32_t i, b, j;
    
  for (i = 0; i < cfont->height; i++)
  {
    b = cfont->data[(ch - 32) * cfont->height + i];
    for (j = 0; j < cfont->width; j++)
    {
      if ((b << j) & 0x8000) 
      {
        OLED_DrawPixel(xpos + j, (ypos + i), WHITE);
      } 
      else 
      {
        OLED_DrawPixel(xpos + j, (ypos + i), BLACK);
      }
    }
  }

  if(screen_update)
  {
	OLED_screen_update();
  }
  
  return ch;
}


int screen_disp_str
(
  uint16_t          xpos,
  uint16_t          ypos,
  char*             str, 
  font_data_type    *cfont
)
{
  currentX = xpos;
  currentY = ypos;
  
  while (*str) 
  {
    if (screen_disp_char(currentX, currentY, *str, cfont, FALSE) != *str)
    {
      return FALSE;
    }

    currentX += cfont->width;
    if(currentX >= screen_x_max)
    {
      currentX = xpos;
      currentY += cfont->height+1;
    }
    str++;
  }
  
  OLED_screen_update();
  
  return TRUE;
}

char screen_disp_num
(
  uint16_t          xpos,
  uint16_t          ypos,
  uint16_t              num, 
  font_data_type    *cfont
)
{
  uint32_t i, b, j,n,m;

  currentX = xpos;
  currentY = ypos;

   /* Check available space in LCD */
  if (OLED1306_WIDTH <= (currentX + cfont->width) ||
      OLED1306_HEIGHT <= (currentY + cfont->height))
  {
    return 0;
  }

  m = num;
  if(num > 9)
  {
    n = num /10;
    for (i = 0; i < cfont->height; i++)
    {
      b = cfont->data[(n + 16) * cfont->height + i];
      for (j = 0; j < cfont->width; j++)
      {
        if ((b << j) & 0x8000) 
        {
          OLED_DrawPixel(currentX + j, currentY + i, WHITE);
        } 
        else 
        {
          OLED_DrawPixel(currentX + j, currentY + i, BLACK);
        }
      }
    }
    currentX += cfont->width;
    m = num%10;
  }
  
  /* Go through font */
  for (i = 0; i < cfont->height; i++)
  {
    b = cfont->data[(m + 16) * cfont->height + i];
    for (j = 0; j < cfont->width; j++)
    {
      if ((b << j) & 0x8000) 
      {
        OLED_DrawPixel(currentX + j, currentY + i, WHITE);
      } 
      else 
      {
        OLED_DrawPixel(currentX + j, currentY + i, BLACK);
      }
    }
  }
  
  /* Increase pointer */
  currentX += cfont->width;
  
  OLED_screen_update();
  return num;
}

#ifdef D_CODE_OPTIMIZE
char screen_disp_char2
(
  uint16_t          xpos,
  uint16_t          ypos,
  char              ch, 
  font_data_type    *cfont,
  uint8_t           screen_update
)
{
  uint32_t i, b, j;
  
  if(ch == ' ')ch=96;
    
  for (i = 0; i < cfont->height; i++)
  {
    b = cfont->data[(ch-96) * cfont->height + i];
    for (j = 0; j < cfont->width; j++)
    {
      if ((b << j) & 0x8000) 
      {
        OLED_DrawPixel(xpos + j, (ypos + i), WHITE);
      } 
      else 
      {
        OLED_DrawPixel(xpos + j, (ypos + i), BLACK);
      }
    }
  }

  if(screen_update)
  {
	OLED_screen_update();
  }
  
  return (ch==96 ? 32:ch);
}


int screen_disp_str2
(
  uint16_t          xpos,
  uint16_t          ypos,
  char*             str, 
  font_data_type    *cfont
)
{
  currentX = xpos;
  currentY = ypos;
  
  while (*str) 
  {
    if (screen_disp_char2(currentX, currentY, *str, cfont, FALSE) != *str)
    {
      return FALSE;
    }
    
    currentX += cfont->width;
    if(currentX >= screen_x_max)
    {
      currentX = xpos;
      currentY += cfont->height+1;
    }
    str++;
  }
  
  OLED_screen_update();
  
  return TRUE;
}
#endif/* D_CODE_OPTIMIZE */

void screen_disp_line
(
  uint16_t     x1, 
  uint16_t     y1, 
  uint16_t     x2, 
  uint16_t     y2
)	
{
  int x, y;
  
  x = ABS(x1 - x2);
  y = ABS(y1 - y2);
  
  if((x == 0) && (y == 0))
  {
   OLED_DrawPixel(x1, y1, WHITE);
  }
  else if(x >= y)
  {
    if(x1 < x2)
    {
      for(x = x1; x <= x2; x++)
      {
        y = y1 + (long)(x - x1)*(long)(y2 - y1)/(x2 - x1);
        OLED_DrawPixel(x, y, WHITE);
      }
    }
    else
    {
      for(x = x1; x >= x2; x--)
      {
      y = y1 + (long)(x - x1)*(long)(y2 - y1)/(x2 - x1);
      OLED_DrawPixel(x, y, WHITE);
      }
    }
  }
  else
  {
    if(y1 < y2)
    {
      for(y = y1; y <= y2; y++)
      {
      x = x1 + (long)(y - y1)*(long)(x2 - x1)/(y2 - y1);
      OLED_DrawPixel(x, y, WHITE);
      }
    }
    else
    {
      for(y = y1; y >= y2; y--)
      {
      x = x1 + (long)(y - y1)*(long)(x2 - x1)/(y2 - y1);
      OLED_DrawPixel(x, y, WHITE);
      }
    }
  }
  
}

void screen_disp_line_remove
(
  uint16_t     x1, 
  uint16_t     y1, 
  uint16_t     x2, 
  uint16_t     y2
)	
{
  int x, y;
  
  x = ABS(x1 - x2);
  y = ABS(y1 - y2);
  
  if((x == 0) && (y == 0))
  {
   OLED_DrawPixel(x1, y1, BLACK);
  }
  else if(x >= y)
  {
    if(x1 < x2)
    {
      for(x = x1; x <= x2; x++)
      {
        y = y1 + (long)(x - x1)*(long)(y2 - y1)/(x2 - x1);
        OLED_DrawPixel(x, y, BLACK);
      }
    }
    else
    {
      for(x = x1; x >= x2; x--)
      {
      y = y1 + (long)(x - x1)*(long)(y2 - y1)/(x2 - x1);
      OLED_DrawPixel(x, y, BLACK);
      }
    }
  }
  else
  {
    if(y1 < y2)
    {
      for(y = y1; y <= y2; y++)
      {
      x = x1 + (long)(y - y1)*(long)(x2 - x1)/(y2 - y1);
      OLED_DrawPixel(x, y, BLACK);
      }
    }
    else
    {
      for(y = y1; y >= y2; y--)
      {
      x = x1 + (long)(y - y1)*(long)(x2 - x1)/(y2 - y1);
      OLED_DrawPixel(x, y, BLACK);
      }
    }
  }
  
}

void screen_disp_rectangle
(
  uint16_t     x1,
  uint16_t     y1, 
  uint16_t     x2,
  uint16_t     y2
)
{
  screen_disp_line(x1, y1, x1, y2);
  screen_disp_line(x2, y1, x2, y2);
  screen_disp_line(x1, y1, x2, y1);
  screen_disp_line(x1, y2, x2, y2);

  OLED_screen_update();
}

void screen_disp_circle
(
  uint16_t     x1,
  uint16_t     y1, 
  uint16_t     r
)
{
  int x, y;
  float s;
  
  for(y = y1 - r*3/4; y <= y1 + r*3/4; y++)
  {
    s = sqrt((long)r*(long)r - (long)(y-y1)*(long)(y-y1)) + 0.5;
    x = x1 + (int)s;
    OLED_DrawPixel(x, y, WHITE);
    x = x1 - (int)s;
    OLED_DrawPixel(x, y, WHITE);
  }
  
  for(x = x1 - r*3/4; x <= x1 + r*3/4; x++)
  {
    s = sqrt((long)r*(long)r - (long)(x-x1)*(long)(x-x1)) + 0.5;
    y = y1 + (int)s;
    OLED_DrawPixel(x, y, WHITE);
    y = y1 - (int)s;
    OLED_DrawPixel(x, y, WHITE);
  }

  OLED_screen_update();
}

#define ONE_ROUND_SCROOL_TIME 16 // 1 round interval

void screen_disp_stop_scroll(void)
{
  OLED_stop_scroll();
}

//right/left scroll smoothly.. And rotate original pos.
void screen_disp_horizon_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round)
{
  if(round)
  {
    for(uint8_t i=0;i<round*ONE_ROUND_SCROOL_TIME;i++)
    {
      OLED_set_horizon_scroll_right(start_page, end_page, start_x, end_x);
      OLED_DELAY(200);
    }
    OLED_stop_scroll();
  }
  else //For Stop, must Call OLED_stop_scroll later...
  {
    OLED_set_horizon_scroll_right(start_page, end_page, start_x, end_x);
  }
}

void screen_disp_horizon_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round)
{
  if(round)
  {
    for(uint8_t i=0;i<round*ONE_ROUND_SCROOL_TIME;i++)
    {
      OLED_set_horizon_scroll_left(start_page, end_page, start_x, end_x);
      OLED_DELAY(200);
    }
    OLED_stop_scroll();
  }
  else
  {
    OLED_set_horizon_scroll_left(start_page, end_page, start_x, end_x);
  }
}

//right/left scroll by 1 colommn tick tick.. And Not rotate original pos.
void screen_disp_content_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round)
{
  if(round)
  {
    for(uint8_t i=0;i<round*ONE_ROUND_SCROOL_TIME;i++)
    {
      OLED_set_content_right_scroll(start_page, end_page, start_x, end_x);
      OLED_DELAY(300);
    }
  }
  else
  {
    OLED_set_content_right_scroll(start_page, end_page, start_x, end_x);
  }
}

void screen_disp_content_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t start_x, uint8_t end_x, uint32_t round)
{
  if(round)
  {
    for(uint8_t i=0;i<round*ONE_ROUND_SCROOL_TIME;i++)
    {
      OLED_set_content_left_scroll(start_page, end_page, start_x, end_x);
      OLED_DELAY(300);
    }
  }
  else
  {
    OLED_set_content_left_scroll(start_page, end_page, start_x, end_x);
  }
}

void screen_disp_vertical_right_left(uint8_t direct, uint8_t start, uint8_t end)
{
  if(direct) //vertical right
  {
    OLED_set_vertical_scroll_right(start, end);
  }
  else //vertical left
  {
    OLED_set_vertical_scroll_right(start, end);
  }
}

void screen_disp_verify_all_page(void)
{
  screen_clear(WHITE);
  OLED_DELAY(500);
  screen_clear(BLACK);
  OLED_DELAY(500);
}

void screen_disp_verify_page(uint8_t start, uint8_t end)
{
  int8_t i;

#ifdef COM_PIN_HW_CONFIG_128_32
  if(end > PAGE3) 
  {
    end = PAGE3;
  }
#else
  if(end > PAGE7) 
  {
    end = PAGE7;
  }
#endif

  OLED_fill(WHITE);
  for(i=start;i<=end;i++) 
  {
    OLED_screen_page_update((OLED_page_type)i);
    //OLED_DELAY(100);
  }

  OLED_fill(BLACK);
  for(i=end;i>=start;i--) 
  {
    OLED_screen_page_update((OLED_page_type)i);
    //OLED_DELAY(100);
  }
}

void screen_disp_change_direction(void)
{
  OLED_change_disp_direction();
}

void screen_disp_change_contrast(uint8_t val)
{
  OLED_change_contrast(val);
}

void screen_disp_backup_screen(void)
{
  OLED_save_screen();
}

void screen_disp_restore_screen(void)
{
  OLED_save_screen_update();
}


#define NEXT_X_POS(order)   (CODE_START_X+((order-1)*(UNDER_LINE_WIDTH+SEPARATOR_WIDTH)))

void screen_disp_code_init(void)
{
  uint8_t start_x = CODE_START_X;
  uint8_t start_y = CODE_START_Y;
  uint8_t lw = UNDER_LINE_WIDTH;
  uint8_t sep = SEPARATOR_WIDTH;
  uint8_t i = 0;
  
  for(i=0; i<CODE_NUMBER; i++)
  {
    screen_disp_line(start_x + i*(lw+sep), start_y, start_x+lw+i*(lw+sep), start_y);
  }
  OLED_screen_update();

}


void screen_disp_code_num(uint8_t order, uint8_t number)
{
  screen_disp_num(NEXT_X_POS(order)+2, CODE_START_Y - 9, number, &Font_7x10);
}

void screen_disp_code_char(uint8_t order, char ch)
{
  screen_disp_char(NEXT_X_POS(order)+2, CODE_START_Y - 9, ch, &Font_7x10, TRUE);
}

void screen_disp_underline(uint8_t order, uint8_t enabled)
{
  if(enabled)
  {
    screen_disp_line(NEXT_X_POS(order), CODE_START_Y,NEXT_X_POS(order)+UNDER_LINE_WIDTH, CODE_START_Y);
  }
  else
  {
    screen_disp_line_remove(NEXT_X_POS(order), CODE_START_Y,NEXT_X_POS(order)+UNDER_LINE_WIDTH, CODE_START_Y);
  }
  OLED_screen_update();
}

void screen_disp_brightness(uint8_t level)
{
  uint8_t oled_level = 0;

  switch(level)
  {
    case 1:
      oled_level = 1;
      break;
    case 2:
      oled_level = 0x10;
      break;
    case 3:
      oled_level = 0x20;
      break;
    case 4:
      oled_level = 0x50;
      break;
    case 5:
      oled_level = 0xFF;
      break;
  }

  OLED_set_contrast(oled_level);
}

void screen_disp_inverse(uint8_t enabled)
{
  OLED_set_normal_or_inverse_display(enabled);
}

