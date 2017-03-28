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
#include "ms.h"

#define MAX_STACK         (256)
#define INIT_GC_THRESHOLD (8)

typedef enum _marked {
  UNMARKED,
  MARKED,
} NyMarked;

struct _vm {
  NyObject* stack[MAX_STACK];
  int stack_cnt;

  NyObject* head_obj;
  int objcnt;
  int maxobj;
};

#define CHECK(cond, msg) do {\
  if (!(cond)) {\
    fprintf(stderr, "%s\n", msg);\
    abort();\
  }\
} while (0)

static void
_NyVM_Push(NyVM* vm, NyObject* obj) {
  CHECK(vm->stack_cnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stack_cnt++] = obj;
}

static NyObject*
_NyVM_Pop(NyVM* vm) {
  CHECK(vm->stack_cnt > 0, "VM stack underflow");
  return vm->stack[--vm->stack_cnt];
}

static void
_NyGC_Mark(NyObject* obj) {
  if (obj->gc.marked == MARKED)
    return;

  obj->gc.marked = MARKED;
  if (obj->type == OBJECT_PAIR) {
    _NyGC_Mark(obj->head);
    _NyGC_Mark(obj->tail);
  }
}

static void
_NyGC_MarkAll(NyVM* vm) {
  for (int i = 0; i < vm->stack_cnt; ++i)
    _NyGC_Mark(vm->stack[i]);
}

static void
_NyGC_Sweep(NyVM* vm) {
  NyObject** start_obj = &vm->head_obj;
  while (*start_obj) {
    if ((*start_obj)->gc.marked == UNMARKED) {
      NyObject* unmarked = *start_obj;
      *start_obj = unmarked->next;
      free(unmarked);
      --vm->objcnt;
    }
    else {
      (*start_obj)->gc.marked = UNMARKED;
      start_obj = &(*start_obj)->next;
    }
  }
}

static NyObject*
_NyObject_New(NyVM* vm, NyType type) {
  if (vm->objcnt >= vm->maxobj)
    NyGC_Collect(vm);

  NyObject* obj = (NyObject*)malloc(sizeof(NyObject));
  obj->gc.marked = UNMARKED;
  obj->type = type;
  obj->next = vm->head_obj;
  vm->head_obj = obj;
  ++vm->objcnt;

  return obj;
}

NyVM*
NyVM_New(void) {
  NyVM* vm = (NyVM*)malloc(sizeof(NyVM));
  CHECK(vm != NULL, "create NyVM failed");

  vm->stack_cnt = 0;
  vm->head_obj = NULL;
  vm->objcnt = 0;
  vm->maxobj = INIT_GC_THRESHOLD;

  return vm;
}

void
NyVM_Free(NyVM* vm) {
  vm->stack_cnt = 0;
  NyGC_Collect(vm);
  free(vm);
}

NyObject*
NyObject_PushInt(NyVM* vm, int value) {
  NyObject* obj = _NyObject_New(vm, OBJECT_INT);
  obj->value = value;
  _NyVM_Push(vm, obj);

  return obj;
}

NyObject*
NyObject_PushPair(NyVM* vm) {
  NyObject* obj = _NyObject_New(vm, OBJECT_PAIR);
  obj->tail = _NyVM_Pop(vm);
  obj->head = _NyVM_Pop(vm);
  _NyVM_Push(vm, obj);

  return obj;
}

NyObject*
NyObject_Pop(NyVM* vm) {
  return _NyVM_Pop(vm);
}

void
NyGC_Collect(NyVM* vm) {
  int old_objcnt = vm->objcnt;

  _NyGC_MarkAll(vm);
  _NyGC_Sweep(vm);

  vm->maxobj = (int)(vm->maxobj * 1.5);
  fprintf(stdout, "NyGC collected [%d] objects, [%d] remaining.\n",
      old_objcnt - vm->objcnt, vm->objcnt);
}
