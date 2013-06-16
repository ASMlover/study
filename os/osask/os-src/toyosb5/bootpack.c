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
#include <string.h>
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
#include "window.h"
#include "console.h"


#define KEYCMD_LED    (0xed)

extern timer_ctrl_t g_timerctl;


void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  fifo32_t fifo, keycmd;
  int  fifobuf[128], keycmd_buf[32];
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
  static char s_keytable[0x80] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    0,   0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 
    0,   0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'','`', 
    0,   '\\','Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,   '*', 
    0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
    0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0
  };
  static char s_keytable_shift[0x80] = {
    0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    0,   0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
    0,   0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"','~', 
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 
    0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
    0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0
  };
  task_t* task_a;
  task_t* task_cons;
  timer_t* timer;
  int key_to = 0, key_shift = 0;
  int key_leds = (binfo->leds >> 4) & 7;  /* CapsLock state */
  int keycmd_wait = -1;
  console_t* console;
  int j, x, y;
  layer_t* layer;


  init_gdt_idt();
  init_pic();
  io_sti();   /* after initialize IDT/PIC, allow all CPU's interruptors */

  fifo_init(&fifo, fifobuf, 128, 0);
  init_pit(); /* initialize programmable interval timer */
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);
  io_out8(PIC0_IMR, 0xf8);  /* set PIT/PIC1/keyboard permission 11111000 */
  io_out8(PIC1_IMR, 0xef);  /* set mouse permission 11101111 */
  fifo_init(&keycmd, keycmd_buf, 32, 0);

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
  *((int*)0x0fe4) = (int)layer_mgr;

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
  task_cons->tss.esp = 
    mem_mgr_alloc_4k(mem_mgr, 64 * 1024) + 64 * 1024 - 12;
  task_cons->tss.eip = (int)&console_task;
  task_cons->tss.es  = 1 * 8;
  task_cons->tss.cs  = 2 * 8;
  task_cons->tss.ss  = 1 * 8;
  task_cons->tss.ds  = 1 * 8;
  task_cons->tss.fs  = 1 * 8;
  task_cons->tss.gs  = 1 * 8;
  *((int*)(task_cons->tss.esp + 4)) = (int)cons_layer;
  *((int*)(task_cons->tss.esp + 8)) = memory_total;
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


  fifo_put(&keycmd, KEYCMD_LED);
  fifo_put(&keycmd, key_leds);

  for ( ; ; ) {
    if (fifo_size(&keycmd) > 0 && keycmd_wait < 0) {
      /* send it when send data to keyboard controller */
      keycmd_wait = fifo_get(&keycmd);
      wait_kbc_sendready();
      io_out8(PORT_KEYDATA, keycmd_wait);
    }

    io_cli();

    if (0 == fifo_size(&fifo)) {
      task_sleep(task_a);
      io_sti();
    }
    else {
      data = fifo_get(&fifo);
      io_sti();

      if (256 <= data && data <= 511) {
        if (data < (256 + 0x80)) {
          /* translation character */
          if (0 == key_shift)
            debug_info[0] = s_keytable[data - 256];
          else 
            debug_info[0] = s_keytable_shift[data - 256];
        }
        else 
          debug_info[0] = 0;
        if ('A' <= debug_info[0] && debug_info[0] <= 'Z') {
          /* it's an english character */
          if ((0 == (key_leds & 4) && 0 == key_shift) 
            || (0 != (key_leds & 4) && 0 != key_shift))
            debug_info[0] += 0x20;  /* character lower ('A' -> 'a') */
        }
        if (0 != debug_info[0]) {
          /* general character */
          if (0 == key_to) {
            /* send character data to TASK-A */
            if (cursor_x < 128) {
              /* display one character and cursor move backward one */
              debug_info[1] = 0;
              drawstring_and_refresh(win_layer, cursor_x, 28, 
                  COLOR8_000000, COLOR8_FFFFFF, debug_info, 1);
              cursor_x += 8;
            }
          }
          else  /* send character data to console window */
            fifo_put(&task_cons->fifo, debug_info[0] + 256);
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
        if ((256 + 0x1c) == data) {
          /* Enter Key */
          if (0 != key_to)  /* send to Console window */
            fifo_put(&task_cons->fifo, 10 + 256);
        }

        if ((256 + 0x0f) == data) {
          /* TAB */
          if (0 == key_to) {
            key_to = 1;
            make_title8(win_buf, win_layer->w_size, "TASK-A", 0);
            make_title8(cons_buf, cons_layer->w_size, "Console", 1);
            cursor_c = -1;  /* don't show the cursor */
            fill_box8(win_layer->buf, win_layer->w_size, 
                COLOR8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);

            fifo_put(&task_cons->fifo, 2);  /* console cursor -> ON */
          }
          else {
            key_to = 0;
            make_title8(win_buf, win_layer->w_size, "TASK-A", 1);
            make_title8(cons_buf, cons_layer->w_size, "Console", 0);
            cursor_c = COLOR8_000000; /* show the cursor */

            fifo_put(&task_cons->fifo, 3);  /* console cursor -> OFF */
          }
          layers_refresh(win_layer, 0, 0, win_layer->w_size, 21);
          layers_refresh(cons_layer, 0, 0, cons_layer->w_size, 21);
        }
        if ((256 + 0x2a) == data) /* left shift down */
          key_shift |= 1;
        if ((256 + 0x36) == data) /* right shift down */
          key_shift |= 2;
        if ((256 + 0xaa) == data) /* left shift up */
          key_shift &= ~1;
        if ((256 + 0xb6) == data) /* right shift up */
          key_shift &= ~2;
        if ((256 + 0x3a) == data) { /* CapsLock */
          key_leds ^= 4;
          fifo_put(&keycmd, KEYCMD_LED);
          fifo_put(&keycmd, key_leds);
        }
        if ((256 + 0x45) == data) { /* NumLock */
          key_leds ^= 2;
          fifo_put(&keycmd, KEYCMD_LED);
          fifo_put(&keycmd, key_leds);
        }
        if ((256 + 0x46) == data) { /* ScrollLock */
          key_leds ^= 1;
          fifo_put(&keycmd, KEYCMD_LED);
          fifo_put(&keycmd, key_leds);
        }
        if (((256 + 0x3e) == data) 
            && (0 != key_shift) 
            && (0 != task_cons->tss.ss0)) {
          /* Shift + F4 force to stop user application */
          console = (console_t*)*((int*)0x0fec);
          console_putstr0(console, "\nbreak(key) :\n");
          io_cli();
          task_cons->tss.eax = (int)&(task_cons->tss.esp0);
          task_cons->tss.eip = (int)asm_stop_user_app;
          io_sti();
        }
        if (((256 + 0x57) == data) && (layer_mgr->top > 2)) {
          /* F11 switch window */
          layer_updown(layer_mgr->layers_addr[1], layer_mgr->top - 1);
        }
        if ((256 + 0xfa) == data)   /* keyboard recevie data success */
          keycmd_wait = -1;
        if ((256 + 0xfe) == data) { /* keyboard recevie data failed */
          wait_kbc_sendready();
          io_out8(PORT_KEYDATA, keycmd_wait);
        }

        /* show cursor again */
        if (cursor_c >= 0) {
          fill_box8(win_layer->buf, win_layer->w_size, 
              cursor_c, cursor_x, 28, cursor_x + 7, 43);
        }
        layers_refresh(win_layer, cursor_x, 28, cursor_x + 8, 44);
      }
      else if (512 <= data && data <= 767) {
        if (0 != mouse_decode(&mdec, data - 512)) {
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

          layer_slide(mouse_layer, mouse_x, mouse_y);

          if (0 != (mdec.state & 0x01)) {
            /* down left click, move window */
            /* find the layer of mouse on */
            for (j = layer_mgr->top - 1; j > 0; --j) {
              layer = layer_mgr->layers_addr[j];
              x = mouse_x - layer->x;
              y = mouse_y - layer->y;
              if (0 <= x && x < layer->w_size 
                  && 0 <= y && y < layer->h_size) {
                if (layer->buf[y * layer->w_size + x] != layer->alpha) {
                  layer_updown(layer, layer_mgr->top - 1);
                  break;
                }
              }
            }
          }
        }
      }
      else if (data <= 1) { /* timer by cursor */
        if (0 != data) {
          timer_init(timer, &fifo, 0);
          if (cursor_c >= 0)
            cursor_c = COLOR8_000000;
        }
        else {
          timer_init(timer, &fifo, 1);
          if (cursor_c >= 0)
            cursor_c = COLOR8_FFFFFF;
        }
        timer_settimer(timer, 50);
        
        if (cursor_c >= 0) {
          fill_box8(win_layer->buf, win_layer->w_size, 
              cursor_c, cursor_x, 28, cursor_x + 7, 43);
          layers_refresh(win_layer, cursor_x, 28, cursor_x + 8, 44);
        }
      }
    }
  }
}

