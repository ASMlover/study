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
#include <stdio.h>
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


typedef struct boot_info_s {
  char cyls, leds, vmode, reserve;
  short screen_x, screen_y;
  unsigned char* vram;
} boot_info_t;

typedef struct segment_descriptor_s {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
} segment_descriptor_t;

typedef struct gate_descriptor_s {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
} gate_descriptor_t;



static void 
set_segment_descriptor(segment_descriptor_t* sd, 
    unsigned int limit, int base, int access_right)
{
  if (limit > 0xfffff) {
    access_right |= 0x8000;
    limit /= 0x1000;
  }
  sd->limit_low     = limit & 0xffff;
  sd->base_low      = base & 0xffff;
  sd->base_mid      = (base >> 16) & 0xff;
  sd->access_right  = access_right & 0xff;
  sd->limit_high    = ((limit >> 16) & 0x0f) | ((access_right >> 8) & 0xf0);
  sd->base_high     = (base >> 24) & 0xff;
}

static void 
set_gate_descriptor(gate_descriptor_t* gd, 
    int offset, int selector, int access_right) 
{
  gd->offset_low    = offset & 0xffff;
  gd->selector      = selector;
  gd->dw_count      = (access_right >> 8) & 0xff;
  gd->access_right  = access_right & 0xff;
  gd->offset_high   = (offset >> 16) & 0xffff;
}

static void 
init_gdt_idt(void)
{
  segment_descriptor_t* gdt = (segment_descriptor_t*)0x00270000;
  gate_descriptor_t* idt = (gate_descriptor_t*)0x0026f800;
  int i;

  /* initialize GDT (13bits 0~8191) */
  for (i = 0; i < 8192; ++i)
    set_segment_descriptor(gdt + i, 0, 0, 0);
  set_segment_descriptor(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
  set_segment_descriptor(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
  load_gdtr(0xffff, 0x00270000);

  /* initialize IDT */
  for (i = 0; i < 256; ++i) 
    set_gate_descriptor(idt + i, 0, 0, 0);
  load_idtr(0x7ff, 0x0026f800);
}

  
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

static void 
init_screen(unsigned char* vram, int w, int h)
{
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
}

static void 
draw_font8(unsigned char* vram, int xsize, int x, int y, char c, char* font)
{
  int i;
  char* p;
  char d;

  for (i = 0; i < 16; ++i) {
    p = vram + (y + i) * xsize + x;
    d = font[i];
    if (0 != (d & 0x80))
      p[0] = c;
    if (0 != (d & 0x40))
      p[1] = c;
    if (0 != (d & 0x20))
      p[2] = c;
    if (0 != (d & 0x10))
      p[3] = c;
    if (0 != (d & 0x08))
      p[4] = c;
    if (0 != (d & 0x04))
      p[5] = c;
    if (0 != (d & 0x02))
      p[6] = c;
    if (0 != (d & 0x01))
      p[7] = c;
  }
}

static void 
draw_font8_asc(unsigned char* vram, int xsize, 
    int x, int y, char color, unsigned char* string)
{
  extern char charlib[4096];
  for ( ; *string != 0x00; ++string) {
    draw_font8(vram, xsize, x, y, color, charlib + *string * 16);
    x += 8;
  }
}

static void 
init_mouse_cursor8(char* mouse, char bg_color)
{
  static char s_cursor[16][16] = {
    "**************..", 
    "*ooooooooooo*...", 
    "*oooooooooo*....", 
    "*ooooooooo*.....", 
    "*oooooooo*......", 
    "*ooooooo*.......", 
    "*ooooooo*.......", 
    "*oooooooo*......", 
    "*oooo**ooo*.....", 
    "*ooo*..*ooo*....", 
    "*oo*....*ooo*...", 
    "*o*......*ooo*..", 
    "**........*ooo*.", 
    "*..........*ooo*", 
    "............*oo*", 
    ".............***",
  };

  int x, y;
  for (y = 0; y < 16; ++y) {
    for (x = 0; x < 16; ++x) {
      switch (s_cursor[y][x]) {
      case '*':
        mouse[y * 16 + x] = COLOR8_000000; break;
      case 'o':
        mouse[y * 16 + x] = COLOR8_FFFFFF; break;
      case '.':
        mouse[y * 16 + x] = bg_color; break;
      }
    }
  }
}

static void 
draw_block8_8(char* vram, int vxsize, int mouse_w, int mouse_h, 
    int x, int y, char* img_addr, int pixels_per_line)
{
  int w, h;
  for (h = 0; h < mouse_h; ++h) {
    for (w = 0; w < mouse_w; ++w) 
      vram[(y + h) * vxsize + (x + w)] = img_addr[h * pixels_per_line + w];
  }
}





void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)0x0ff0;
  char debug_info[64], mouse_cursor[256];
  int mouse_x, mouse_y;

  init_gdt_idt();
  init_palette();
  init_screen(binfo->vram, binfo->screen_x, binfo->screen_y);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;
  init_mouse_cursor8(mouse_cursor, COLOR8_848484);
  draw_block8_8(binfo->vram, binfo->screen_x, 16, 16, 
      mouse_x, mouse_y, mouse_cursor, 16);

  draw_font8_asc(binfo->vram, binfo->screen_x, 8, 8, 
      COLOR8_FFFFFF, "HELLO BOYS:");
  draw_font8_asc(binfo->vram, binfo->screen_x, 9, 9, 
      COLOR8_000000, "HELLO BOYS:");
  draw_font8_asc(binfo->vram, binfo->screen_x, 31, 31, 
      COLOR8_000000, "WELCOME TO THE LOVELY TOY-OS.");
  draw_font8_asc(binfo->vram, binfo->screen_x, 30, 30, 
      COLOR8_FFFFFF, "WELCOME TO THE LOVELY TOY-OS.");

  sprintf(debug_info, "screen=>{%d, %d}", binfo->screen_x, binfo->screen_y);
  draw_font8_asc(binfo->vram, binfo->screen_x, 16, 64, 
      COLOR8_FF0000, debug_info);

  for ( ; ; )
    io_hlt();
}
