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

#include "common.h"

#define COLOR8_000000   (0)
#define COLOR8_FF0000   (1)
#define COLOR8_00FF00   (2)
#define COLOR8_FFFF00   (3)
#define COLOR8_0000FF   (4)
#define COLOR8_FF00FF   (5)
#define COLOR8_00FFFF   (6)
#define COLOR8_FFFFFF   (7)
#define COLOR8_C6C6C6   (8)
#define COLOR8_840000   (9)
#define COLOR8_008400   (10)
#define COLOR8_848400   (11)
#define COLOR8_000084   (12)
#define COLOR8_840084   (13)
#define COLOR8_008484   (14)
#define COLOR8_848484   (15)


  
static void 
set_palette(int start, int end, unsigned char* rgb)
{
  int i, eflags;

  eflags = io_load_eflags();    /* value of interrupt flag */
  io_cli();                     /* set flag = 0, disabled interrupt */
  io_out8(0x03c8, start);
  for (i = start; i <= end; ++i) {
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);
}

static void 
init_palette(void)
{
  static unsigned char s_table_rgb[16 * 3] = {
    0x00, 0x00, 0x00, /* 0: black */
    0xff, 0x00, 0x00, /* 1: bright red */
    0x00, 0xff, 0x00, /* 2: bright green */
    0xff, 0xff, 0x00, /* 3: bright yellow */
    0x00, 0x00, 0xff, /* 4: bright blue */
    0xff, 0x00, 0xff, /* 5: bright purple */
    0x00, 0xff, 0xff, /* 6: shallow bright blue */
    0xff, 0xff, 0xff, /* 7: white */
    0xc6, 0xc6, 0xc6, /* 8: bright grey */
    0x84, 0x00, 0x00, /* 9: dark red */
    0x00, 0x84, 0x00, /* 10: dark green */
    0x84, 0x84, 0x00, /* 11: dark yellow */
    0x00, 0x00, 0x84, /* 12: dark blue */
    0x84, 0x00, 0x84, /* 13: dark purple */
    0x00, 0x84, 0x84, /* 14: shallow dark blue */
    0x84, 0x84, 0x84, /* 15: shallow grey */
  };

  set_palette(0, 15, s_table_rgb);
}

static void 
fill_box8(unsigned char* vram, int xsize, unsigned char color, 
    int left, int top, int right, int bottom)
{
  int x, y;

  for (y = top; y <= bottom; ++y) {
    for (x = left; x <= right; ++x)
      vram[y * xsize + x] = color;
  }
}



void 
HariMain(void)
{
  char* vram;
  int w, h;

  init_palette();
  vram = (char*)0xa0000;   /* setting address */
  w = 320;
  h = 200;

  fill_box8(vram, w, COLOR8_848484, 0, 0, w - 1, h - 29);
  fill_box8(vram, w, COLOR8_C6C6C6, 0, h - 28, w - 1, h - 28);
  fill_box8(vram, w, COLOR8_FFFFFF, 0, h - 27, w - 1, h - 27);
  fill_box8(vram, w, COLOR8_C6C6C6, 0, h - 26, w - 1, h - 1);

  fill_box8(vram, w, COLOR8_FFFFFF, 3, h - 24, 59, h - 24);
  fill_box8(vram, w, COLOR8_FFFFFF, 2, h - 24, 2, h - 4);
  fill_box8(vram, w, COLOR8_848484, 3, h - 4, 59, h - 4);
  fill_box8(vram, w, COLOR8_848484, 59, h - 23, 59, h - 5);
  fill_box8(vram, w, COLOR8_000000, 2, h - 3, 59, h - 3);
  fill_box8(vram, w, COLOR8_000000, 60, h - 24, 60, h - 3);

  fill_box8(vram, w, COLOR8_848484, w - 47, h - 24, w - 4, h - 24);
  fill_box8(vram, w, COLOR8_848484, w - 47, h - 23, w - 47, h - 4);
  fill_box8(vram, w, COLOR8_FFFFFF, w - 47, h - 3, w - 4, h - 3);
  fill_box8(vram, w, COLOR8_FFFFFF, w - 3, h - 24, w - 3, h - 3);

  for ( ; ; )
    io_hlt();
}
