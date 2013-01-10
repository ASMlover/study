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
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
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
#include <stdlib.h>
#include <time.h>
#include "../lib/queue.h"
#include "bank_queuing.h"


#define EVENT_ENTER       (0)
#define EVENT_LEAVE       (1)
#define TOTAL_WORK_TIME   (60 * 8)  /* min */
#define BANK_WINDOWS      (8)

typedef struct {
  time_t enter_time;
  time_t trade_time;
} *bank_customer_t;



static int 
get_shortest_queue(queue_t* q) 
{
  int i, min, len, min_queue = 0;
  min = queue_size(q[0]);
  for (i = 1; i < BANK_WINDOWS; ++i) {
    len = queue_size(q[i]);
    if (min > len) {
      min = len;
      min_queue = i;
    }
  }

  return min_queue;
}


static void 
solve_bank_business(int business, queue_t q, bank_customer_t customer, 
  time_t time_iter, time_t* total_time, int* total_customers)
{
  switch (business) {
  case EVENT_ENTER: 
    queue_enqueue(q, customer);
    break;
  case EVENT_LEAVE:
    {
      bank_customer_t leave = (bank_customer_t)queue_dequeue(q);
      *total_customers += 1;
      *total_time += time_iter - leave->enter_time;

      free(leave);
    } break;
  }
}


void 
bank_queuing(void)
{
  int i;
  time_t time_iter = 0;
  int enter_new_custome = 0;
  int enter_customers = 0;
  int total_customers = 0;
  time_t total_delay_time = 0;
  queue_t bank_win[BANK_WINDOWS];

  for (i = 0; i < BANK_WINDOWS; ++i)
    bank_win[i] = queue_create();

  srand(time(0));
  while (time_iter < TOTAL_WORK_TIME) {
    enter_new_custome = rand() % 2;
    if (enter_new_custome) {
      int q_idx;
      bank_customer_t customer = (bank_customer_t)malloc(sizeof(*customer));
      customer->enter_time = time_iter;
      customer->trade_time = rand() % 10;

      q_idx = get_shortest_queue(bank_win);
      solve_bank_business(EVENT_ENTER, 
        bank_win[q_idx], customer, time_iter, NULL, NULL);
      ++enter_customers;
    }

    for (i = 0; i < BANK_WINDOWS; ++i) {
      if (queue_empty(bank_win[i]))
        continue;

      bank_customer_t c = (bank_customer_t)queue_peek(bank_win[i]);
      if (time_iter - c->enter_time == c->trade_time)
        solve_bank_business(EVENT_LEAVE, 
          bank_win[i], NULL, time_iter, 
          &total_delay_time, &total_customers);
    }

    ++time_iter;
  }

  for (i = 0; i < BANK_WINDOWS; ++i) {
    while (!queue_empty(bank_win[i])) 
      free(queue_dequeue(bank_win[i]));
    queue_delete(&bank_win[i]);
  }

  fprintf(stdout, 
    "total time : %ld, enter customer: %d, "
    "total customer : %d, avg time : %lf\n", 
    total_delay_time, enter_customers, total_customers, 
    (double)total_delay_time / (double)total_customers);
}
