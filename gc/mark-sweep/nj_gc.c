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
#include "nj_gc.h"

#define MAX_STACK         (256)
#define INIT_GC_THRESHOLD (8)

typedef enum _marked {
  UNMARKED,
  MARKED,
} NjMarked;

struct _vm {
  NjObject* stack[MAX_STACK];
  int stackcnt;

  NjObject* startobj;
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
_njord_push(NjVM* vm, NjObject* obj) {
  CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njord_pop(NjVM* vm) {
  CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static void
_njord_gc_mark(NjObject* obj) {
  if (obj->gc.marked == MARKED)
    return;

  obj->gc.marked = MARKED;
  if (obj->type == OBJECT_PAIR) {
    _njord_gc_mark(obj->head);
    _njord_gc_mark(obj->tail);
  }
}

static void
_njord_gc_markall(NjVM* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njord_gc_mark(vm->stack[i]);
}

static void
_njord_gc_sweep(NjVM* vm) {
  NjObject** start_obj = &vm->startobj;
  while (*start_obj) {
    if ((*start_obj)->gc.marked == UNMARKED) {
      NjObject* unmarked = *start_obj;
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

static NjObject*
_njord_new_object(NjVM* vm, NjType type) {
  if (vm->objcnt >= vm->maxobj)
    njord_collect(vm);

  NjObject* obj = (NjObject*)malloc(sizeof(NjObject));
  obj->gc.marked = UNMARKED;
  obj->type = type;
  obj->next = vm->startobj;
  vm->startobj = obj;
  ++vm->objcnt;

  return obj;
}

NjVM*
njord_new(void) {
  NjVM* vm = (NjVM*)malloc(sizeof(NjVM));
  CHECK(vm != NULL, "create NjVM failed");

  vm->stackcnt = 0;
  vm->startobj = NULL;
  vm->objcnt = 0;
  vm->maxobj = INIT_GC_THRESHOLD;

  return vm;
}

void
njord_free(NjVM* vm) {
  vm->stackcnt = 0;
  njord_collect(vm);
  free(vm);
}

NjObject*
njord_pushint(NjVM* vm, int value) {
  NjObject* obj = _njord_new_object(vm, OBJECT_INT);
  obj->value = value;
  _njord_push(vm, obj);

  return obj;
}

NjObject*
njord_pushpair(NjVM* vm) {
  NjObject* obj = _njord_new_object(vm, OBJECT_PAIR);
  obj->tail = _njord_pop(vm);
  obj->head = _njord_pop(vm);
  _njord_push(vm, obj);

  return obj;
}

NjObject*
njord_pop(NjVM* vm) {
  return _njord_pop(vm);
}

void
njord_collect(NjVM* vm) {
  int old_objcnt = vm->objcnt;

  _njord_gc_markall(vm);
  _njord_gc_sweep(vm);

  vm->maxobj = (int)(vm->maxobj * 1.5);
  fprintf(stdout, "njord collected [%d] objects, [%d] remaining.\n",
      old_objcnt - vm->objcnt, vm->objcnt);
}
