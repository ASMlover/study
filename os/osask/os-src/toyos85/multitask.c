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




static task_t* 
task_now(void)
{
  task_level_t* task_lv = &g_taskctrl->levels[g_taskctrl->now_lv];
  return task_lv->tasks_map[task_lv->now];
}

static void 
task_add(task_t* task)
{
  task_level_t* task_lv = &g_taskctrl->levels[task->level];
  task_lv->tasks_map[task_lv->running] = task;
  ++task_lv->running;
  task->flags = 2;  /* activity */
}

static void 
task_del(task_t* task)
{
  int i;
  task_level_t* task_lv = &g_taskctrl->levels[task->level];

  /* search the position of task */
  for (i = 0; i < task_lv->running; ++i) {
    if (task_lv->tasks_map[i] == task)
      break;
  }

  --task_lv->running;
  if (i < task_lv->now)
    --task_lv->now;
  if (task_lv->now >= task_lv->running)
    task_lv->now = 0;

  task->flags = 1;  /* sleep */

  for ( ; i < task_lv->running; ++i)
    task_lv->tasks_map[i] = task_lv->tasks_map[i + 1];
}

static void 
task_switch_sub(void)
{
  int i;

  /* find the top level */
  for (i = 0; i < MAX_TASKLEVELS; ++i) {
    if (g_taskctrl->levels[i].running > 0)
      break;
  }

  g_taskctrl->now_lv = i;
  g_taskctrl->change_lv = 0;
}




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

  for (i = 0; i < MAX_TASKLEVELS; ++i) {
    g_taskctrl->levels[i].running = 0;
    g_taskctrl->levels[i].now     = 0;
  }

  task = task_alloc();
  task->flags    = 2; /* flags about activity */
  task->level    = 0; /* top task level */
  task->priority = 2; /* 0.02 s */
  task_add(task);
  task_switch_sub();  /* setting level */
  load_tr(task->sel);
  g_task_timer = timer_alloc();
  timer_settimer(g_task_timer, task->priority);

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
task_run(task_t* task, int level, int priority)
{
  if (level < 0)
    level = task->level;
  if (priority > 0)
    task->priority = priority;

  /* modify the task in activity */
  if (2 == task->flags && task->level != level) 
    task_del(task); /* task->flags will be 1 */

  if (2 != task->flags) {
    task->level = level;
    task_add(task);
  }

  g_taskctrl->change_lv = 1;  /* it will check level at next switch task */
}

void 
task_switch(void)
{
  task_level_t* task_lv = &g_taskctrl->levels[g_taskctrl->now_lv];
  task_t* new_task;
  task_t* now_task = task_lv->tasks_map[task_lv->now];

  ++task_lv->now;
  if (task_lv->now == task_lv->running)
    task_lv->now = 0;

  if (0 != g_taskctrl->change_lv) {
    task_switch_sub();
    task_lv = &g_taskctrl->levels[g_taskctrl->now_lv];
  }
  new_task = task_lv->tasks_map[task_lv->now];
  timer_settimer(g_task_timer, new_task->priority);
  if (new_task != now_task)
    farjump(0, new_task->sel);
}

void 
task_sleep(task_t* task)
{
  task_t* now_task;

  if (2 == task->flags) { /* the task in the wake state */
    now_task = task_now();
    task_del(task);
    if (task == now_task) {
      /* if task sleep, then need to switch task */
      task_switch_sub();
      now_task = task_now();
      farjump(0, now_task->sel);
    }
  }
}
