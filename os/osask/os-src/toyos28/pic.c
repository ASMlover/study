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
#include "pic.h"


#define PORT_KEYDATA    (0x0060)




void 
init_pic(void)
{
  io_out8(PIC0_IMR, 0xff);    /* disabled all interruptors */
  io_out8(PIC1_IMR, 0xff);    /* disabled all interruptors */

  io_out8(PIC0_ICW1, 0x11);   /* edge trigegr mode */
  io_out8(PIC0_ICW2, 0x20);   /* IRQ0~7 received by INT20~27 */
  io_out8(PIC0_ICW3, 1 << 2); /* PIC1 connected by IRQ2 */
  io_out8(PIC0_ICW4, 0x01);   /* no-cached mode */

  io_out8(PIC1_ICW1, 0x11);   /* edge trigegr mode */
  io_out8(PIC1_ICW2, 0x28);   /* IRQ8~15 received by INT28~2f */
  io_out8(PIC1_ICW3, 2);      /* PIC1 connected by IRQ2 */
  io_out8(PIC1_ICW4, 0x01);   /* no-cached mode */

  io_out8(PIC0_IMR, 0xfb);    /* 11111011 disabled outside PIC1 */
  io_out8(PIC1_IMR, 0xff);    /* 11111111 disabled all interruptors */
}

void 
interrupt_handler21(int* esp)
{
  /* interruptors from PS/2 keyboard */
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  unsigned char data, str[4];

  io_out8(PIC0_OCW2, 0x61);
  data = io_in8(PORT_KEYDATA);

  sprintf(str, "%02X", data);
  fill_box8(binfo->vram, binfo->screen_x, 
      COLOR8_008484, 0, 16, 15, 31);
  draw_font8_asc(binfo->vram, binfo->screen_x, 0, 16, COLOR8_FFFFFF, str);
}

void 
interrupt_handler2c(int* esp)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  fill_box8(binfo->vram, binfo->screen_x, 
      COLOR8_000000, 0, 0, 32 * 8 - 1, 15);
  draw_font8_asc(binfo->vram, binfo->screen_x, 0, 0, 
      COLOR8_FFFFFF, "INT 21 (IRQ - 12): PS/2 mouse");

  for ( ; ; )
    io_hlt();
}

void 
interrupt_handler27(int* esp)
{
  io_out8(PIC0_OCW2, 0x67);
}
