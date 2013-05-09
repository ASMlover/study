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


/* mouse decode */
typedef struct mouse_dec_s {
  unsigned char buf[3];
  unsigned char phase;
  int x, y;
  int state;  /* state of mouse click */
} mouse_dec_t;



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
enable_mouse(mouse_dec_t* mdec)
{
  /* make mouse activity */
  wait_kbc_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_kbc_sendready();
  io_out8(PORT_KEYDATA, MOUSECMD_ENABLE);

  /* ACK(0xfa) will be recevied if successful */
  mdec->phase = 0;  /* phase of waitting for 0xfa */
}

static int 
mouse_decode(mouse_dec_t* mdec, unsigned char data)
{
  if (0 == mdec->phase) {
    /* phase of waitting for 0xfa */
    if (0xfa == data)
      mdec->phase = 1;

    return 0;
  }
  else if (1 == mdec->phase) {
    /* waitting for the first byte code of mouse */
    if (0x08 == (data & 0xc8)) {
      mdec->buf[0] = data;
      mdec->phase = 2;
    }

    return 0;
  }
  else if (2 == mdec->phase) {
    /* waitting for the second byte code of mouse */
    mdec->buf[1] = data;
    mdec->phase = 3;

    return 0;
  }
  else if (3 == mdec->phase) {
    /* waitting for the third byte code of mouse */
    mdec->buf[2] = data;
    mdec->phase = 1;

    mdec->state = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];
    if (0 != (mdec->buf[0] & 0x10))
      mdec->x |= 0xffffff00;
    if (0 != (mdec->buf[0] & 0x20))
      mdec->y |= 0xffffff00;
    mdec->y = -mdec->y;

    return 1;
  }

  return -1;
}



void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  char debug_info[64], mouse_cursor[256], keybuf[32], mousebuf[128];
  int mouse_x, mouse_y;
  int data;
  mouse_dec_t mdec;

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
  sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
  draw_font8_asc(binfo->vram, binfo->screen_x, 0, 0, 
      COLOR8_FFFFFF, debug_info);


  enable_mouse(&mdec);   /* enabled mouse */

  for ( ; ; ) {
    io_cli();

    if (0 == fifo_size(&g_keybuf) && 0 == fifo_size(&g_mousebuf)) 
      io_stihlt();
    else {
      if (0 != fifo_size(&g_keybuf)) {
          data = fifo_get(&g_keybuf);

          io_sti();
          sprintf(debug_info, "%02X", data);
          fill_box8(binfo->vram, binfo->screen_x, 
            COLOR8_848484, 0, 16, 15, 31);
          draw_font8_asc(binfo->vram, binfo->screen_x, 
            0, 16, COLOR8_FFFFFF, debug_info);
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
          fill_box8(binfo->vram, binfo->screen_x, 
              COLOR8_848484, 32, 16, 32 + 15 * 8 - 1, 31);
          draw_font8_asc(binfo->vram, binfo->screen_x, 
              32, 16, COLOR8_FFFFFF, debug_info);

          /* move mouse pointer */
          fill_box8(binfo->vram, binfo->screen_x, COLOR8_848484, 
              mouse_x, mouse_y, mouse_x + 15, mouse_y + 15);/* hide mouse */
          mouse_x += mdec.x;
          mouse_y += mdec.y;

          if (mouse_x < 0)
            mouse_x = 0;
          if (mouse_y < 0)
            mouse_y = 0;
          if (mouse_x > binfo->screen_x - 16)
            mouse_x = binfo->screen_x - 16;
          if (mouse_y > binfo->screen_y - 16)
            mouse_y = binfo->screen_y - 16;

          sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
          fill_box8(binfo->vram, binfo->screen_x, 
              COLOR8_848484, 0, 0, 79, 15);     /* hide mouse position */
          draw_font8_asc(binfo->vram, binfo->screen_x, 
              0, 0, COLOR8_FFFFFF, debug_info); /* show mouse position */
          draw_block8_8(binfo->vram, binfo->screen_x, 
              16, 16, mouse_x, mouse_y, mouse_cursor,  16); /* draw mouse */
        }
      }
    }
  }
}

