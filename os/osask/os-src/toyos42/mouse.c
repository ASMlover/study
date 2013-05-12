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
#include "pic.h"
#include "fifo.h"
#include "keyboard.h"
#include "mouse.h"


fifo8_t g_mousebuf;


#define KEYCMD_SENDTO_MOUSE     (0xd4)
#define MOUSECMD_ENABLE         (0xf4)

  
void 
interrupt_handler2c(int* esp)
{
  unsigned char data;

  io_out8(PIC1_OCW2, 0x64);   /* IRQ-12 finished (PIC1) */
  io_out8(PIC0_OCW2, 0x62);   /* IRQ-02 finished (PIC0) */
  data = io_in8(PORT_KEYDATA);

  fifo_put(&g_mousebuf, data);
}

void 
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

int 
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
