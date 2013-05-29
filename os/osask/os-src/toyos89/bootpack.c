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
#include "multitask.h"


extern timer_ctrl_t g_timerctl;


static void 
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

static void 
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

static void 
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

static void 
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

static void 
console_task(layer_t* layer)
{
#define INPUT_BEG_POS (24)
#define PROMPT_CHAR   ("> ")
#define PROMPT_LEN    (2)
  timer_t* timer;
  task_t*  task = task_now();
  int data, fifobuf[128];
  int cursor_x = INPUT_BEG_POS, cursor_c = COLOR8_000000;
  char buf[2];
  
  fifo_init(&task->fifo, fifobuf, 128, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settimer(timer, 50);

  /* display the prompt of console window */
  drawstring_and_refresh(layer, 8, 28, 
      COLOR8_FFFFFF, COLOR8_000000, PROMPT_CHAR, PROMPT_LEN);

  for ( ; ; ) {
    io_cli();

    if (0 == fifo_size(&task->fifo)) {
      task_sleep(task);
      io_sti();
    }
    else {
      data = fifo_get(&task->fifo);
      io_sti();

      if (data <= 1) {
        /* timer for cursor */
        if (0 != data) {
          timer_init(timer, &task->fifo, 0);
          cursor_c = COLOR8_FFFFFF;
        }
        else {
          timer_init(timer, &task->fifo, 1);
          cursor_c = COLOR8_000000;
        }

        timer_settimer(timer, 50);
      }
      if (256 <= data && data <= 511) {
        /* keyboard data */
        if ((8 + 256) == data) {
          /* backspace */
          if (cursor_x > INPUT_BEG_POS) {
            /* erase the cursor and move forward one character */
            drawstring_and_refresh(layer, cursor_x, 28, 
                COLOR8_FFFFFF, COLOR8_000000, " ", 1);
            cursor_x -= 8;
          }
        }
        else {
          /* general character */
          if (cursor_x < 240) {
            /* display one character and move backward one character */
            buf[0] = data - 256;
            buf[1] = 0;
            drawstring_and_refresh(layer, cursor_x, 28, 
                COLOR8_FFFFFF, COLOR8_000000, buf, 1);
            cursor_x += 8;
          }
        }
      }
      /* show the cursor again */
      fill_box8(layer->buf, layer->w_size, 
          cursor_c, cursor_x, 28, cursor_x + 7, 43);
      layers_refresh(layer, cursor_x, 28, cursor_x + 8, 44);
    }
  }
}


void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  fifo32_t fifo;
  int  fifobuf[128];
  char debug_info[64];
  int mouse_x, mouse_y, cursor_x, cursor_c;
  int data;
  mouse_dec_t mdec;
  unsigned int memory_total;
  mem_mgr_t* mem_mgr = (mem_mgr_t*)MEMMGR_ADDR;
  layer_mgr_t* layer_mgr;
  layer_t* back_layer;
  layer_t* mouse_layer;
  layer_t* win_layer;
  layer_t* cons_layer;
  unsigned char* back_buf;
  unsigned char  mouse_buf[256];
  unsigned char* win_buf;
  unsigned char* cons_buf;
  static char s_keytable[0x54] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    0,   0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 
    0,   0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'','`', 
    0,   '\\','Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,   '*', 
    0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
    0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
  };
  task_t* task_a;
  task_t* task_cons;
  timer_t* timer;
  int key_to = 0;


  init_gdt_idt();
  init_pic();
  io_sti();   /* after initialize IDT/PIC, allow all CPU's interruptors */

  fifo_init(&fifo, fifobuf, 128, 0);
  init_pit(); /* initialize programmable interval timer */
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);
  io_out8(PIC0_IMR, 0xf8);  /* set PIT/PIC1/keyboard permission 11111000 */
  io_out8(PIC1_IMR, 0xef);  /* set mouse permission 11101111 */

  memory_total = memory_test(0x00400000, 0xbfffffff);
  mem_mgr_init(mem_mgr);
  mem_mgr_free(mem_mgr, 0x00001000, 0x0009e000);  /*0x00001000~0x0009e000*/
  mem_mgr_free(mem_mgr, 0x00400000, memory_total - 0x00400000);

  init_palette();
  layer_mgr = layer_mgr_init(mem_mgr, binfo->vram, 
      binfo->screen_x, binfo->screen_y);
  task_a = task_init(mem_mgr);
  fifo.task = task_a;
  task_run(task_a, 1, 2);

  /* back layer */
  back_layer = layer_alloc(layer_mgr);
  back_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 
      binfo->screen_x * binfo->screen_y);
  layer_setbuf(back_layer, back_buf, 
      binfo->screen_x, binfo->screen_y, -1);
  init_screen(back_buf, binfo->screen_x, binfo->screen_y);

  /* console task */
  cons_layer = layer_alloc(layer_mgr);
  cons_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 256 * 165);
  layer_setbuf(cons_layer, cons_buf, 256, 165, -1); /* transparent */
  make_window8(cons_buf, 256, 165, "Console", 0);
  make_text8(cons_layer, 8, 28, 240, 128, COLOR8_000000);
  task_cons = task_alloc();
  task_cons->tss.esp = mem_mgr_alloc_4k(mem_mgr, 64 * 1024) + 64 * 1024 - 8;
  task_cons->tss.eip = (int)&console_task;
  task_cons->tss.es  = 1 * 8;
  task_cons->tss.cs  = 2 * 8;
  task_cons->tss.ss  = 1 * 8;
  task_cons->tss.ds  = 1 * 8;
  task_cons->tss.fs  = 1 * 8;
  task_cons->tss.gs  = 1 * 8;
  *((int*)(task_cons->tss.esp + 4)) = (int)cons_layer;
  task_run(task_cons, 2, 2);  /* level = 2, priority = 2 */
  
  /* window layer */
  win_layer = layer_alloc(layer_mgr);
  win_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 160 * 52);
  layer_setbuf(win_layer, win_buf, 144, 52, -1);
  make_window8(win_buf, 144, 52, "TASK-A", 1);
  make_text8(win_layer, 8, 28, 128, 16, COLOR8_FFFFFF);
  cursor_x = 8;
  cursor_c = COLOR8_FFFFFF;
  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settimer(timer, 50);

  /* mouse layer */
  mouse_layer = layer_alloc(layer_mgr);
  layer_setbuf(mouse_layer, mouse_buf, 16, 16, 99);
  init_mouse_cursor8(mouse_buf, 99);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;

  layer_slide(back_layer, 0, 0);
  layer_slide(cons_layer, 32, 4);
  layer_slide(win_layer, 64, 56);
  layer_slide(mouse_layer, mouse_x, mouse_y);
  layer_updown(back_layer, 0);
  layer_updown(cons_layer, 1);
  layer_updown(win_layer, 2);
  layer_updown(mouse_layer, 3);
  sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
  drawstring_and_refresh(back_layer, 0, 0, 
      COLOR8_FFFFFF, COLOR8_848484, debug_info, 10);

  sprintf(debug_info, "memory total: %dMB, free space: %dKB", 
      memory_total / (1024 * 1024), mem_mgr_total(mem_mgr) / 1024);
  drawstring_and_refresh(back_layer, 0, 32, 
      COLOR8_FFFFFF, COLOR8_848484, debug_info, 40);


  for ( ; ; ) {
    io_cli();

    if (0 == fifo_size(&fifo)) {
      task_sleep(task_a);
      io_sti();
    }
    else {
      data = fifo_get(&fifo);
      io_sti();

      if (256 <= data && data <= 511) {
        sprintf(debug_info, "%02X", data - 256);
        drawstring_and_refresh(back_layer, 0, 16, 
            COLOR8_FFFFFF, COLOR8_848484, debug_info, 2);

        if (data < (256 + 0x54) && 0 != s_keytable[data - 256]) {
          /* general character */
          if (0 == key_to) {
            /* character for TASK-A */
            if (cursor_x < 128) {
              /* normal character, show 1 character, move cursor 1 time */
              debug_info[0] = s_keytable[data - 256];
              debug_info[1] = 0;
              drawstring_and_refresh(win_layer, cursor_x, 28, 
                  COLOR8_000000, COLOR8_FFFFFF, debug_info, 1);
              cursor_x += 8;
            }
          }
          else /* send to console window */
            fifo_put(&task_cons->fifo, s_keytable[data - 256] + 256);
        }
        if ((256 + 0x0e) == data) {
          /* backspace, recover cursor by sapce, move back cursor 1 time */
          if (0 == key_to) {
            if (cursor_x > 8) {
              drawstring_and_refresh(win_layer, cursor_x, 28, 
                  COLOR8_000000, COLOR8_FFFFFF, " ", 1);
              cursor_x -= 8;
            }
          }
          else 
            fifo_put(&task_cons->fifo, 8 + 256);
        }

        if ((256 + 0x0f) == data) {
          /* TAB */
          if (0 == key_to) {
            key_to = 1;
            make_title8(win_buf, win_layer->w_size, "TASK-A", 0);
            make_title8(cons_buf, cons_layer->w_size, "Console", 1);
          }
          else {
            key_to = 0;
            make_title8(win_buf, win_layer->w_size, "TASK-A", 1);
            make_title8(cons_buf, cons_layer->w_size, "Console", 0);
          }
          layers_refresh(win_layer, 0, 0, win_layer->w_size, 21);
          layers_refresh(cons_layer, 0, 0, cons_layer->w_size, 21);
        }

        /* show cursor again */
        fill_box8(win_layer->buf, win_layer->w_size, 
            cursor_c, cursor_x, 28, cursor_x + 7, 43);
        layers_refresh(win_layer, cursor_x, 28, cursor_x + 8, 44);
      }
      else if (512 <= data && data <= 767) {
        if (0 != mouse_decode(&mdec, data - 512)) {
          /* show all mouse bytes code */
          sprintf(debug_info, "[lcr %4d %4d]", mdec.x, mdec.y);
          if (0 != (mdec.state & 0x01)) 
            debug_info[1] = 'L';
          if (0 != (mdec.state & 0x02))
            debug_info[3] = 'R';
          if (0 != (mdec.state & 0x04))
            debug_info[2] = 'C';
          drawstring_and_refresh(back_layer, 32, 16, 
              COLOR8_FFFFFF, COLOR8_848484, debug_info, 15);

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
          drawstring_and_refresh(back_layer, 0, 0, 
              COLOR8_FFFFFF, COLOR8_848484, debug_info, 10);
          layer_slide(mouse_layer, mouse_x, mouse_y);

          if (0 != (mdec.state & 0x01)) {
            /* down left click, move window */
            layer_slide(win_layer, mouse_x - 80, mouse_y - 8);
          }
        }
      }
      else if (data <= 1) { /* timer by cursor */
        if (0 != data) {
          timer_init(timer, &fifo, 0);
          cursor_c = COLOR8_000000;
        }
        else {
          timer_init(timer, &fifo, 1);
          cursor_c = COLOR8_FFFFFF;
        }
        timer_settimer(timer, 50);
        fill_box8(win_layer->buf, win_layer->w_size, 
            cursor_c, cursor_x, 28, cursor_x + 7, 43);
        layers_refresh(win_layer, cursor_x, 28, cursor_x + 8, 44);
      }
    }
  }
}

