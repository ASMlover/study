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


#define PIT_CTRL            (0x0043)
#define PIT_CNT0            (0x0040)

#define TIMER_FLAGS_ALLOC   (1)   /* configure statue has been setted */
#define TIMER_FLAGS_USING   (2)   /* timer is running */


timer_ctrl_t g_timerctl;

void 
init_pit(void)
{
  int i;
  timer_t* timer;

  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);

  g_timerctl.count = 0;
  for (i = 0; i < MAX_TIMER; ++i)
    g_timerctl.timers[i].flags = 0; /* unused */

  timer = timer_alloc();  /* get one */
  timer->timeout = 0xffffffff;
  timer->flags   = TIMER_FLAGS_USING;
  timer->next    = 0; /* the end one */
  g_timerctl.timer0 = timer;
  g_timerctl.next   = 0xffffffff;
}

void 
interrupt_handler20(int* esp)
{
  timer_t* timer;

  io_out8(PIC0_OCW2, 0x60); /* send sign to PIC from IRQ-00 */
  ++g_timerctl.count;
  if (g_timerctl.next > g_timerctl.count)
    return;   /* not the next timeout timer, break */

  timer = g_timerctl.timer0;
  for ( ; ; ) {
    /* timers are all using, donot need to check flags */
    if (timer->timeout > g_timerctl.count)
      break;

    /* timeout */
    timer->flags = TIMER_FLAGS_ALLOC;
    fifo_put(timer->fifo, timer->data);
    timer = timer->next;
  }

  g_timerctl.timer0 = timer;
  g_timerctl.next   = timer->timeout;
}



timer_t* 
timer_alloc(void)
{
  int i;

  for (i = 0; i < MAX_TIMER; ++i) {
    if (0 == g_timerctl.timers[i].flags) {
      g_timerctl.timers[i].flags = TIMER_FLAGS_ALLOC;
      return &g_timerctl.timers[i];
    }
  }

  return 0;
}

void 
timer_free(timer_t* timer)
{
  timer->flags = 0; /* unused */
}

void 
timer_init(timer_t* timer, fifo32_t* fifo, int data)
{
  timer->fifo = fifo;
  timer->data = data;
}

void 
timer_settimer(timer_t* timer, unsigned int timeout)
{
  int eflags;
  timer_t* t; /* timers iterator */
  timer_t* s; /* prev-timer before t */

  timer->timeout = timeout + g_timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  eflags = io_load_eflags();
  io_cli();

  t = g_timerctl.timer0;
  if (timer->timeout <= t->timeout) {
    /* insert into the front */
    g_timerctl.timer0 = timer;
    timer->next = t;
    g_timerctl.next = timer->timeout;
    io_store_eflags(eflags);
    
    return;
  }

  /* search position to be inserted */
  for ( ; ; ) {
    s = t;
    t = t->next;
    
    if (timer->timeout <= t->timeout) {
      /* insert into between s and t */
      s->next = timer;
      timer->next = t;
      io_store_eflags(eflags);

      return;
    }
  }
}
