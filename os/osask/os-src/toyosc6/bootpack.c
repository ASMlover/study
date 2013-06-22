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



static void 
keywin_on(layer_t* key_win)
{
  change_wtitle8(key_win, 1);
  if (0 != (key_win->flags & 0x20))
    fifo_put(&key_win->task->fifo, 2);  /* cursor of command window on */
}

static void 
keywin_off(layer_t* key_win)
{
  change_wtitle8(key_win, 0);
  if (0 != (key_win->flags & 0x20))
    fifo_put(&key_win->task->fifo, 3);  /* cursor of command window off */
}



void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  fifo32_t fifo, keycmd;
  int  fifobuf[128], keycmd_buf[32];
  int* cons_fifo[2];
  char debug_info[64];
  int mouse_x, mouse_y;
  int data;
  mouse_dec_t mdec;
  unsigned int memory_total;
  mem_mgr_t* mem_mgr = (mem_mgr_t*)MEMMGR_ADDR;
  layer_mgr_t* layer_mgr;
  layer_t* back_layer;
  layer_t* mouse_layer;
  layer_t* cons_layer[2];
  unsigned char* back_buf;
  unsigned char  mouse_buf[256];
  unsigned char* cons_buf[2];
  static char s_keytable[0x80] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    0x08,0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 
    0x0a,0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'','`', 
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
    0x08,0,   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
    0x0a,0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"','~', 
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 
    0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
    0,   '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0
  };
  task_t* task_a;
  task_t* task_cons[2];
  task_t* task;
  int key_shift = 0;
  int key_leds = (binfo->leds >> 4) & 7;  /* CapsLock state */
  int keycmd_wait = -1;
  int i, j, x, y;
  int movemode_x = -1, movemode_y = -1;
  layer_t* layer = 0;
  layer_t* key_win;


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
  for (i = 0; i < 2; ++i) {
    cons_layer[i] = layer_alloc(layer_mgr);
    cons_buf[i] = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 256 * 165);
    layer_setbuf(cons_layer[i], cons_buf[i], 256, 165, -1);/* transparent */
    make_window8(cons_buf[i], 256, 165, "Console", 0);
    make_text8(cons_layer[i], 8, 28, 240, 128, COLOR8_000000);
    task_cons[i] = task_alloc();
    task_cons[i]->tss.esp = 
      mem_mgr_alloc_4k(mem_mgr, 64 * 1024) + 64 * 1024 - 12;
    task_cons[i]->tss.eip = (int)&console_task;
    task_cons[i]->tss.es  = 1 * 8;
    task_cons[i]->tss.cs  = 2 * 8;
    task_cons[i]->tss.ss  = 1 * 8;
    task_cons[i]->tss.ds  = 1 * 8;
    task_cons[i]->tss.fs  = 1 * 8;
    task_cons[i]->tss.gs  = 1 * 8;
    *((int*)(task_cons[i]->tss.esp + 4)) = (int)cons_layer[i];
    *((int*)(task_cons[i]->tss.esp + 8)) = memory_total;
    task_run(task_cons[i], 2, 2);  /* level = 2, priority = 2 */
    cons_layer[i]->task = task_cons[i];
    cons_layer[i]->flags |= 0x20;
    cons_fifo[i] = (int*)mem_mgr_alloc_4k(mem_mgr, 128 * 4);
    fifo_init(&task_cons[i]->fifo, cons_fifo[i], 128, task_cons[i]);
  }
  
  /* mouse layer */
  mouse_layer = layer_alloc(layer_mgr);
  layer_setbuf(mouse_layer, mouse_buf, 16, 16, 99);
  init_mouse_cursor8(mouse_buf, 99);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;

  layer_slide(back_layer, 0, 0);
  layer_slide(cons_layer[1], 56, 6);
  layer_slide(cons_layer[0], 8, 2);
  layer_slide(mouse_layer, mouse_x, mouse_y);
  layer_updown(back_layer, 0);
  layer_updown(cons_layer[1], 1);
  layer_updown(cons_layer[0], 2);
  layer_updown(mouse_layer, 3);
  key_win = cons_layer[0];
  keywin_on(key_win);


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

      if (0 == key_win->flags) {
        /* closed input window */
        key_win = layer_mgr->layers_addr[layer_mgr->top - 1];
        keywin_on(key_win);
      }

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
        if (0 != debug_info[0]) /* normal character, backspace, Enter */
          fifo_put(&key_win->task->fifo, debug_info[0] + 256);
        if ((256 + 0x0f) == data) {
          /* TAB */
          keywin_off(key_win);
          j = key_win->height - 1;
          if (0 == j)
            j = layer_mgr->top - 1;
          key_win = layer_mgr->layers_addr[j];
          keywin_on(key_win);
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
        if (((256 + 0x3e) == data) && (0 != key_shift)) {
          /* F4 -> 0x3e */
          task = key_win->task;
          if (0 != task && 0 != task->tss.ss0) {
            /* Shift + F4 force to stop user application */
            console_putstr0(task->cons, "\nbreak(key) :\n");
            io_cli();
            task->tss.eax = (int)&(task->tss.esp0);
            task->tss.eip = (int)asm_stop_user_app;
            io_sti();
          }
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
            /* left click button down */
            if (movemode_x < 0) {
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
                    if (layer != key_win) {
                      keywin_off(key_win);
                      key_win = layer;
                      keywin_on(key_win);
                    }
                    if (3 <= x 
                        && x < layer->w_size - 3 && 3 <= y && y < 21) {
                      movemode_x = mouse_x;
                      movemode_y = mouse_y;
                    }
                    if (layer->w_size - 21 <= x && x < layer->w_size - 5 
                        && 5 <= y && y < 19) {
                      /* click close button of the window */
                      if (0 != (layer->flags & 0x10)) {
                        /* whether it's the window of application */
                        task = layer->task;
                        console_putstr0(task->cons, "\nbreak(mouse) : \n");
                        io_cli();
                        task->tss.eax = (int)&task->tss.esp0;
                        task->tss.eip = (int)asm_stop_user_app;
                        io_sti();
                      }
                    }
                    break;
                  }
                }
              }
            }
            else {
              /* on move mode */
              x = mouse_x - movemode_x;
              y = mouse_y - movemode_y;
              layer_slide(layer, layer->x + x, layer->y + y);
              movemode_x = mouse_x;
              movemode_y = mouse_y;
            }
          }
          else 
            movemode_x = -1;  /* return to normal mode */
        }
      }
    }
  }
}

