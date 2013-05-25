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
#include "desc_tbl.h"
#include "timer.h"
#include "multitask.h"


task_ctrl_t* g_taskctrl;
timer_t* g_task_timer;



task_t* 
task_init(mem_mgr_t* mem_mgr) 
{
  int i;
  task_t* task;
  segment_descriptor_t* gdt = (segment_descriptor_t*)ADR_GDT;
  g_taskctrl = (task_ctrl_t*)mem_mgr_alloc_4k(mem_mgr, sizeof(task_ctrl_t));
  for (i = 0; i < MAX_TASKS; ++i) {
    g_taskctrl->tasks[i].flags = 0;
    g_taskctrl->tasks[i].sel   = (TASK_GDT0 + i) * 8;
    set_segment_descriptor(gdt + TASK_GDT0 + i, 
        103, (int)&g_taskctrl->tasks[i].tss, AR_TSS32);
  }

  task = task_alloc();
  task->flags = 2;  /* flags about activity */
  g_taskctrl->running = 1;
  g_taskctrl->now = 0;
  g_taskctrl->tasks_map[0] = task;
  load_tr(task->sel);
  g_task_timer = timer_alloc();
  timer_settimer(g_task_timer, 2);

  return task;
}

task_t* 
task_alloc(void)
{
  int i;
  task_t* task;

  for (i = 0; i < MAX_TASKS; ++i) {
    if (0 == g_taskctrl->tasks[i].flags) {
      task = &g_taskctrl->tasks[i];
      task->flags = 1;  /* flags about using */
      task->tss.eflags = 0x00000202;  /* IF = 1 */
      task->tss.eax = 0;
      task->tss.ecx = 0;
      task->tss.edx = 0;
      task->tss.ebx = 0;
      task->tss.ebp = 0;
      task->tss.esi = 0;
      task->tss.edi = 0;
      task->tss.es  = 0;
      task->tss.ds  = 0;
      task->tss.fs  = 0;
      task->tss.gs  = 0;
      task->tss.ldtr  = 0;
      task->tss.iomap = 0x40000000;

      return task;
    }
  }

  return 0; /* all tasks are using */
}

void 
task_run(task_t* task)
{
  task->flags = 2;  /* task is activity */
  g_taskctrl->tasks_map[g_taskctrl->running] = task;
  ++g_taskctrl->running;
}

void 
task_switch(void)
{
  timer_settimer(g_task_timer, 2);

  if (g_taskctrl->running >= 2) {
    ++g_taskctrl->now;
    if (g_taskctrl->now == g_taskctrl->running)
      g_taskctrl->now = 0;

    farjump(0, g_taskctrl->tasks_map[g_taskctrl->now]->sel);
  }
}
