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
#ifndef __TIMER_HEADER_H__
#define __TIMER_HEADER_H__ 

#include "fifo.h"

#define MAX_TIMER   (500)


typedef struct timer_t {
  struct timer_t* next;
  unsigned int timeout;
  char flags;
  char flags2;
  fifo32_t* fifo;
  int data;
} timer_t;

typedef struct timer_ctrl_t {
  unsigned int count;
  unsigned int next;
  timer_t* timer0;
  timer_t  timers[MAX_TIMER];
} timer_ctrl_t;

extern void init_pit(void);
extern void interrupt_handler20(int* esp);

extern timer_t* timer_alloc(void);
extern void timer_free(timer_t* timer);
extern void timer_init(timer_t* timer, fifo32_t* fifo, int data);
extern void timer_settimer(timer_t* timer, unsigned int timeout);
extern int timer_cancel(timer_t* timer);
extern void timer_cancelall(fifo32_t* fifo);

#endif  /* __TIMER_HEADER_H__ */
