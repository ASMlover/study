/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#include "gc_impl.h"

#define MAX_STACK        (1024)
#define COPYGC_HALF_SIZE (512 << 10)
#define Nj_ASGC(ob) ((GCHead*)(ob) - 1)
#define Nj_OBN(ob)  (((NjVarObject*)(ob))->ob_size + sizeof(GCHead))

static Nj_uchar_t* copymem = NULL;
static Nj_uchar_t* fromspace = NULL;
static Nj_uchar_t* tospace = NULL;
static Nj_uchar_t* allocptr = NULL;

typedef struct _gc {
  Nj_uchar_t* forward;
} GCHead;

typedef struct _vm {
  NjObject_HEAD;

  NjObject* stack[MAX_STACK];
  int stackcnt;
} NjVMObject;

static void njcopy_collect();

static void
_copymem_init(void) {
  copymem = (Nj_uchar_t*)malloc(2 * COPYGC_HALF_SIZE);
  Nj_CHECK(copymem != NULL, "allocate copying space failed");

  tospace = copymem;
  fromspace = tospace + COPYGC_HALF_SIZE;
}

static void*
_copymem_alloc(Nj_ssize_t n) {
  if (allocptr + n > tospace + COPYGC_HALF_SIZE)
    njcopy_collect();
  Nj_CHECK(allocptr + n <= tospace + COPYGC_HALF_SIZE, "allocate failed");

  void* p = allocptr;
  allocptr += n;
  return p;
}

static void
_njcopy_push(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njcopy_pop(NjVMObject* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static NjObject*
_njcopy_copy(NjObject* obj) {
}

NjTypeObject NjCopy_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "copy_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  0, /* tp_gc */
};
