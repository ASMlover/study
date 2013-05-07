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



extern fifo8_t g_keybuf;
extern fifo8_t g_mousebuf;

#define PORT_KEYDATA            (0x0060)
#define PORT_KEYSTATE           (0x0064)
#define PORT_KEYCMD             (0x0064)
#define KEYSTATE_SEND_NOTREADY  (0x02)
#define KEYCMD_WRITE_MODE       (0x60)
#define KBC_MODE                (0x47)

#define KEYCMD_SENDTO_MOUSE     (0xd4)
#define MOUSECMD_ENABLE         (0xf4)


static void 
wait_kbc_sendready(void)
{
  /* wait keybufyboard code send ready */
  for ( ; ; ) {
    if (0 == (io_in8(PORT_KEYSTATE) & KEYSTATE_SEND_NOTREADY))
      break;
  }
}

static void 
init_keyboard(void)
{
  /* initialize keyboard */
  wait_kbc_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_kbc_sendready();
  io_out8(PORT_KEYDATA, KBC_MODE);
}

static void 
enable_mouse(void)
{
  /* make mouse activity */
  wait_kbc_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_kbc_sendready();
  io_out8(PORT_KEYDATA, MOUSECMD_ENABLE);
}



void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  char debug_info[64], mouse_cursor[256], keybuf[32], mousebuf[128];
  int mouse_x, mouse_y;

  init_gdt_idt();
  init_pic();
  io_sti();

  fifo_init(&g_keybuf, keybuf, 32);
  fifo_init(&g_mousebuf, mousebuf, 128);
  io_out8(PIC0_IMR, 0xf9);
  io_out8(PIC1_IMR, 0xef);

  init_keyboard();    /* initialize keyboard */

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

  enable_mouse();   /* enabled mouse */

  for ( ; ; ) {
    int data;

    io_cli();

    if (0 == fifo_size(&g_keybuf) && 0 == fifo_size(&g_mousebuf)) 
      io_stihlt();
    else {
      if (0 != fifo_size(&g_keybuf)) {
          data = fifo_get(&g_keybuf);

          io_sti();
          sprintf(debug_info, "%02X", data);
          fill_box8(binfo->vram, binfo->screen_x, 
            COLOR8_008484, 0, 16, 15, 31);
          draw_font8_asc(binfo->vram, binfo->screen_x, 
            0, 16, COLOR8_FFFFFF, debug_info);
      }
      else if (0 != fifo_size(&g_mousebuf)) {
        data = fifo_get(&g_mousebuf);

        io_sti();
        sprintf(debug_info, "%02X", data);
        fill_box8(binfo->vram, binfo->screen_x, 
          COLOR8_008484, 32, 16, 47, 31);
        draw_font8_asc(binfo->vram, binfo->screen_x, 
          32, 16, COLOR8_FFFFFF, debug_info);
      }
    }
  }
}

