/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "graphic.h"
#include "window.h"



void 
drawstring_and_refresh(layer_t* layer, int x, int y, 
    int color, int bg_color, char* string, int len)
{
  /*
   * x, y => position of string 
   * color => color of string
   * bg_color => background color of string 
   * string => string to display
   * len => length of string
   */

  fill_box8(layer->buf, layer->w_size, bg_color, 
      x, y, x + len * 8 - 1, y + 15);
  draw_font8_asc(layer->buf, layer->w_size, x, y, color, string);
  layers_refresh(layer, x, y, x + len * 8, y + 16);
}

void 
make_title8(unsigned char* buf, int width, char* title, char act)
{
  static char s_close_btn[14][16] = {
    "OOOOOOOOOOOOOOO@", 
    "OQQQQQQQQQQQQQ$@", 
    "OQQQQQQQQQQQQQ$@", 
    "OQQQ@@QQQQ@@QQ$@", 
    "OQQQQ@@QQ@@QQQ$@", 
    "OQQQQQ@@@@QQQQ$@", 
    "OQQQQQQ@@QQQQQ$@", 
    "OQQQQQ@@@@QQQQ$@", 
    "OQQQQ@@QQ@@QQQ$@", 
    "OQQQ@@QQQQ@@QQ$@", 
    "OQQQQQQQQQQQQQ$@", 
    "OQQQQQQQQQQQQQ$@", 
    "O$$$$$$$$$$$$$$@", 
    "@@@@@@@@@@@@@@@@", 
  };
  int x, y;
  char c, tc, tbc;

  if (0 != act) {
    tc  = COLOR8_FFFFFF;
    tbc = COLOR8_000084;
  }
  else {
    tc  = COLOR8_C6C6C6;
    tbc = COLOR8_848484;
  }

  fill_box8(buf, width, tbc, 3, 3, width - 4, 20);
  draw_font8_asc(buf, width, 24, 4, tc, title);

  for (y = 0; y < 14; ++y) {
    for (x = 0; x < 16; ++x) {
      c = s_close_btn[y][x];
      switch (c) {
      case '@':
        c = COLOR8_000000; break;
      case '$':
        c = COLOR8_848484; break;
      case 'Q':
        c = COLOR8_C6C6C6; break;
      default:
        c = COLOR8_FFFFFF; break;
      }
      buf[(5 + y) * width + (width - 21 + x)] = c;
    }
  }
}

void 
make_window8(unsigned char* buf, int width, 
    int height, char* title, char act)
{
  fill_box8(buf, width, COLOR8_C6C6C6, 0, 0, width - 1, 0);
  fill_box8(buf, width, COLOR8_FFFFFF, 1, 1, width - 2, 1);
  fill_box8(buf, width, COLOR8_C6C6C6, 0, 0, 0, height - 1);
  fill_box8(buf, width, COLOR8_FFFFFF, 1, 1, 1, height - 2);
  fill_box8(buf, width, COLOR8_848484, width - 2, 1, width - 2, height - 2);
  fill_box8(buf, width, COLOR8_000000, width - 1, 0, width - 1, height - 1);
  fill_box8(buf, width, COLOR8_C6C6C6, 2, 2, width - 3, height - 3);
  fill_box8(buf, width, COLOR8_848484, 
      1, height - 2, width - 2, height - 2);
  fill_box8(buf, width, COLOR8_000000, 
      0, height - 1, width - 1, height - 1);

  make_title8(buf, width, title, act);
}

void 
make_text8(layer_t* layer, int x, int y, int w, int h, int c)
{
  /*
   * x, y => position (left, top)
   * w    => size of text window 
   * h    => height of text window 
   * c    => color 
   */
  int x1 = x + w, y1 = y + h;

  fill_box8(layer->buf, layer->w_size, COLOR8_848484, 
      x - 2, y - 3, x1 + 1, y - 3);
  fill_box8(layer->buf, layer->w_size, COLOR8_848484, 
      x - 3, y - 3, x - 3, y1 + 1);
  fill_box8(layer->buf, layer->w_size, COLOR8_FFFFFF, 
      x - 3, y1 + 2, x1 + 1, y1 + 2);
  fill_box8(layer->buf, layer->w_size, COLOR8_FFFFFF, 
      x1 + 2, y - 3, x1 + 2, y1 + 2);
  fill_box8(layer->buf, layer->w_size, COLOR8_000000, 
      x - 1, y - 2, x1 + 0, y - 2);
  fill_box8(layer->buf, layer->w_size, COLOR8_000000, 
      x - 2, y - 2, x - 2, y1 + 0);
  fill_box8(layer->buf, layer->w_size, COLOR8_C6C6C6, 
      x - 2, y1 + 1, x1 + 0, y1 + 1);
  fill_box8(layer->buf, layer->w_size, COLOR8_C6C6C6, 
      x1 + 1, y - 2, x1 + 1, y1 + 1);
  fill_box8(layer->buf, layer->w_size, c, 
      x - 1, y - 1, x1 + 0, y1 + 0);
}
