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
#ifndef __MULTITASK_HEADER_H__
#define __MULTITASK_HEADER_H__ 

#include "memory.h"

#define MAX_TASKS       (1000)    /* max tasks number */
#define TASK_GDT0       (3)       /* alloc to tss from gdt number */
#define MAX_TASKS_LV    (100)     /* max tasks per level */
#define MAX_TASKLEVELS  (10)      /* max levels */

/* task status segment */
typedef struct tss32_t {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
} tss32_t;

typedef struct task_t {
  int sel;    /* selector of gdt */
  int flags;
  int level;
  int priority;
  tss32_t tss;
} task_t;

typedef struct task_level_t {
  int running;  /* number of tasks are running */
  int now;      /* current running task */
  task_t* tasks_map[MAX_TASKS_LV];
} task_level_t;

typedef struct task_ctrl_t {
  int now_lv;     /* current level activity */
  char change_lv; /* whether change the level next switch task */
  task_level_t levels[MAX_TASKLEVELS];
  task_t  tasks[MAX_TASKS];
} task_ctrl_t;

extern task_t* task_init(mem_mgr_t* mem_mgr);
extern task_t* task_alloc(void);
extern void task_run(task_t* task, int level, int priority);
extern void task_switch(void);
extern void task_sleep(task_t* task);

#endif  /* __MULTITASK_HEADER_H__ */
