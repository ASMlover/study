/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "memory.h"



static void def_error_handler(KL_MemController* controller, 
    const char* filename, int line, const char* msg);


static KL_MemController s_def_controller = {
  NULL, /* stderr */
  def_error_handler, 
  MEM_FAIL_AND_EXIT, 
  NULL
};
KL_MemController* g_def_mem_controller = &s_def_controller;


typedef union {
  long    l_dummy;
  double  d_dummy;
  void*   p_dummy;
} KL_Align;

#define KL_MARK_SIZE    (4)

typedef struct KL_MemHeaderStruct {
  /* struct type of memory header */
  int           size;
  char*         filename;
  int           line;
  KL_MemHeader* prev;
  KL_MemHeader* next;
  unsigned char mark[KL_MARK_SIZE];
} KL_MemHeaderStruct;

#define KL_ALIGN_SIZE           (sizeof(KL_Align))
#define KL_revalue_up_align(v)  ((v) ? (((v) - 1) / KL_ALIGN_SIZE + 1) : 0)
#define KL_HEADER_ALIGN_SIZE\
  (KL_revalue_up_align(sizeof(KL_MemHeaderStruct)))
#define KL_MARK                 (0xCD)


union KL_MemHeader {
  /* memory header difinition */
  KL_MemHeaderStruct  s;
  KL_Align            u[KL_HEADER_ALIGN_SIZE];
};



static void 
def_error_handler(KL_MemController* controller, 
    const char* filename, int line, const char* msg)
{
  fprintf(controller->err_file, 
      "MEMORY:%s failed in %s at %d\n", msg, filename, line);
}

static void 
error_handler(KL_MemController* controller, 
    const char* filename, int line, const char* msg)
{
  if (NULL == controller->err_file)
    controller->err_file = stderr;
  controller->err_handler(controller, filename, line, msg);

  if (MEM_FAIL_AND_EXIT == controller->fail_mode)
    exit(1);
}





KL_MemController* 
KL_mem_create_controller(void)
{
  KL_MemController* controller = KL_malloc(sizeof(*controller));
  *controller = s_def_controller;

  return controller;
}

void* 
KL_mem_malloc_func(KL_MemController* controller, 
    const char* filename, int line, size_t size)
{
  void* ptr = malloc(size);
  if (NULL == ptr) 
    error_handler(controller, filename, line, "malloc");

  return ptr;
}


