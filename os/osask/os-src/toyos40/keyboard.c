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


fifo8_t g_keybuf;


#define PORT_KEYSTATE           (0x0064)
#define KEYSTATE_SEND_NOTREADY  (0x02)
#define KEYCMD_WRITE_MODE       (0x60)
#define KBC_MODE                (0x47)


void 
interrupt_handler21(int* esp)
{
  unsigned char data;

  io_out8(PIC0_OCW2, 0x61); /* call PIC that IRQ-01 is finished */
  data = io_in8(PORT_KEYDATA);

  fifo_put(&g_keybuf, data);
}


void 
wait_kbc_sendready(void)
{
  /* wait keybufyboard code send ready */
  for ( ; ; ) {
    if (0 == (io_in8(PORT_KEYSTATE) & KEYSTATE_SEND_NOTREADY))
      break;
  }
}

void 
init_keyboard(void)
{
  /* initialize keyboard */
  wait_kbc_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_kbc_sendready();
  io_out8(PORT_KEYDATA, KBC_MODE);
}
