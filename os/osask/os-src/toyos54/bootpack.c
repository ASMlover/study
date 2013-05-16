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
#include "boot.h"
#include "graphic.h"
#include "desc_tbl.h"
#include "pic.h"
#include "fifo.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"
#include "layer.h"
#include "timer.h"


extern fifo8_t g_keybuf;
extern fifo8_t g_mousebuf;




static void 
make_window8(unsigned char* buf, int width, int height, char* title)
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
  char c;

  fill_box8(buf, width, COLOR8_C6C6C6, 0, 0, width - 1, 0);
  fill_box8(buf, width, COLOR8_FFFFFF, 1, 1, width - 2, 1);
  fill_box8(buf, width, COLOR8_C6C6C6, 0, 0, 0, height - 1);
  fill_box8(buf, width, COLOR8_FFFFFF, 1, 1, 1, height - 2);
  fill_box8(buf, width, COLOR8_848484, width - 2, 1, width - 2, height - 2);
  fill_box8(buf, width, COLOR8_000000, width - 1, 0, width - 1, height - 1);
  fill_box8(buf, width, COLOR8_C6C6C6, 2, 2, width - 3, height - 3);
  fill_box8(buf, width, COLOR8_000084, 3, 3, width - 4, 20);
  fill_box8(buf, width, COLOR8_848484, 
      1, height - 2, width - 2, height - 2);
  fill_box8(buf, width, COLOR8_000000, 
      0, height - 1, width - 1, height - 1);
  draw_font8_asc(buf, width, 24, 4, COLOR8_FFFFFF, title);

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
        c = COLOR8_FFFFFF;
      }
      buf[(5 + y) * width + (width - 21 + x)] = c;
    }
  }
}



void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  char debug_info[64], keybuf[32], mousebuf[128];
  int mouse_x, mouse_y;
  int data;
  mouse_dec_t mdec;
  unsigned int memory_total, count = 0;
  mem_mgr_t* mem_mgr = (mem_mgr_t*)MEMMGR_ADDR;
  layer_mgr_t* layer_mgr;
  layer_t* back_layer;
  layer_t* mouse_layer;
  layer_t* win_layer;
  unsigned char* back_buf;
  unsigned char  mouse_buf[256];
  unsigned char* win_buf;

  init_gdt_idt();
  init_pic();
  io_sti();   /* after initialize IDT/PIC, allow all CPU's interruptors */

  fifo_init(&g_keybuf, keybuf, 32);
  fifo_init(&g_mousebuf, mousebuf, 128);
  init_pit(); /* initialize programmable interval timer */
  io_out8(PIC0_IMR, 0xf8);  /* set PIT/PIC1/keyboard permission 11111000 */
  io_out8(PIC1_IMR, 0xef);  /* set mouse permission 11101111 */

  init_keyboard();      /* initialize keyboard */
  enable_mouse(&mdec);  /* enabled mouse */

  memory_total = memory_test(0x00400000, 0xbfffffff);
  mem_mgr_init(mem_mgr);
  mem_mgr_free(mem_mgr, 0x00001000, 0x0009e000);  /*0x00001000~0x0009e000*/
  mem_mgr_free(mem_mgr, 0x00400000, memory_total - 0x00400000);

  init_palette();
  layer_mgr = layer_mgr_init(mem_mgr, binfo->vram, 
      binfo->screen_x, binfo->screen_y);
  back_layer = layer_alloc(layer_mgr);
  mouse_layer = layer_alloc(layer_mgr);
  win_layer = layer_alloc(layer_mgr);
  back_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 
      binfo->screen_x * binfo->screen_y);
  win_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 160 * 52);
  layer_setbuf(back_layer, back_buf, 
      binfo->screen_x, binfo->screen_y, -1);
  layer_setbuf(mouse_layer, mouse_buf, 16, 16, 99);
  layer_setbuf(win_layer, win_buf, 160, 52, -1);
  init_screen(back_buf, binfo->screen_x, binfo->screen_y);
  init_mouse_cursor8(mouse_buf, 99);
  make_window8(win_buf, 160, 52, "Counter");
  layer_slide(back_layer, 0, 0);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;
  layer_slide(mouse_layer, mouse_x, mouse_y);
  layer_slide(win_layer, 80, 72);
  layer_updown(back_layer, 0);
  layer_updown(win_layer, 1);
  layer_updown(mouse_layer, 2);
  sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
  draw_font8_asc(back_buf, binfo->screen_x, 0, 0, 
      COLOR8_FFFFFF, debug_info);

  sprintf(debug_info, "memory total: %dMB, free space: %dKB", 
      memory_total / (1024 * 1024), mem_mgr_total(mem_mgr) / 1024);
  draw_font8_asc(back_buf, binfo->screen_x, 
      0, 32, COLOR8_FFFFFF, debug_info);

  layers_refresh(back_layer, 0, 0, binfo->screen_x, 48);

  for ( ; ; ) {
    sprintf(debug_info, "%010d", ++count);
    fill_box8(win_buf, 160, COLOR8_C6C6C6, 40, 28, 119, 43);
    draw_font8_asc(win_buf, 160, 40, 28, COLOR8_000000, debug_info);
    layers_refresh(win_layer, 40, 28, 120, 44);

    io_cli();
    if (0 == fifo_size(&g_keybuf) && 0 == fifo_size(&g_mousebuf)) 
      io_stihlt();
    else {
      if (0 != fifo_size(&g_keybuf)) {
          data = fifo_get(&g_keybuf);

          io_sti();
          sprintf(debug_info, "%02X", data);
          fill_box8(back_buf, binfo->screen_x, 
            COLOR8_848484, 0, 16, 15, 31);
          draw_font8_asc(back_buf, binfo->screen_x, 
            0, 16, COLOR8_FFFFFF, debug_info);

          layers_refresh(back_layer, 0, 16, 16, 32);
      }
      else if (0 != fifo_size(&g_mousebuf)) {
        data = fifo_get(&g_mousebuf);
        io_sti();

        if (0 != mouse_decode(&mdec, data)) {
          /* show all mouse bytes code */
          sprintf(debug_info, "[lcr %4d %4d]", mdec.x, mdec.y);
          if (0 != (mdec.state & 0x01)) 
            debug_info[1] = 'L';
          if (0 != (mdec.state & 0x02))
            debug_info[3] = 'R';
          if (0 != (mdec.state & 0x04))
            debug_info[2] = 'C';
          fill_box8(back_buf, binfo->screen_x, 
              COLOR8_848484, 32, 16, 32 + 15 * 8 - 1, 31);
          draw_font8_asc(back_buf, binfo->screen_x, 
              32, 16, COLOR8_FFFFFF, debug_info);
          layers_refresh(back_layer, 32, 16, 32 + 15 * 8, 32);

          mouse_x += mdec.x;
          mouse_y += mdec.y;

          if (mouse_x < 0)
            mouse_x = 0;
          if (mouse_y < 0)
            mouse_y = 0;
          if (mouse_x > binfo->screen_x - 1)
            mouse_x = binfo->screen_x - 1;
          if (mouse_y > binfo->screen_y - 1)
            mouse_y = binfo->screen_y - 1;

          sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
          fill_box8(back_buf, binfo->screen_x, 
              COLOR8_848484, 0, 0, 79, 15);     /* hide mouse position */
          draw_font8_asc(back_buf, binfo->screen_x, 
              0, 0, COLOR8_FFFFFF, debug_info); /* show mouse position */

          layers_refresh(back_layer, 0, 0, 80, 16);
          layer_slide(mouse_layer, mouse_x, mouse_y);
        }
      }
    }
  }
}

