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
#include "timer.h"


/* programmable interval timer */


#define PIT_CTRL    (0x0043)
#define PIT_CNT0    (0x0040)


timer_ctrl_t g_timerctl;

void 
init_pit(void)
{
  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);

  g_timerctl.count   = 0;
  g_timerctl.timeout = 0;
}

void 
interrupt_handler20(int* esp)
{
  io_out8(PIC0_OCW2, 0x60); /* send sign to PIC from IRQ-00 */
  ++g_timerctl.count;

  if (g_timerctl.timeout > 0) { /* if setted timeout */
    --g_timerctl.timeout;
    if (0 == g_timerctl.timeout) 
      fifo_put(g_timerctl.fifo, g_timerctl.data);
  }
}

void 
set_timer(unsigned int timeout, fifo8_t* fifo, unsigned char data)
{
  int eflags = io_load_eflags();
  io_cli();

  g_timerctl.timeout = timeout;
  g_timerctl.fifo = fifo;
  g_timerctl.data = data;

  io_store_eflags(eflags);
}
