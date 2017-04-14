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
#include "njmem.h"

#define MAX_STACK         (1024)
#define INIT_GC_THRESHOLD (64)
#define MAX_GC_THRESHOLD  (1024)
#define Nj_ASGC(ob)       ((GCHead*)(ob) - 1)

typedef enum _marked {
  UNMARKED,
  MARKED,
} NjMarked;

typedef struct _gc {
  Nj_uchar_t marked;
} GCHead;

typedef struct _vm {
  NjObject_HEAD;

  NjObject* stack[MAX_STACK];
  int stackcnt;

  NjObject* startobj;
  int objcnt;
  int maxobj;
} NjVMObject;

static void
_njmarks_push(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njmarks_pop(NjVMObject* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static void
_njmarks_mark(NjObject* obj) {
  if (Nj_ASGC(obj)->marked == MARKED)
    return;

  Nj_ASGC(obj)->marked = MARKED;
  if (obj->ob_type == &NjPair_Type) {
    NjObject* head = njord_pairgetter(obj, "head");
    _njmarks_mark(head);
    NjObject* tail = njord_pairgetter(obj, "tail");
    _njmarks_mark(tail);
  }
}

static void
_njmarks_mark_all(NjVMObject* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njmarks_mark(vm->stack[i]);
}

static void
_njmarks_sweep(NjVMObject* vm) {
  NjObject** startobj = &vm->startobj;
  while (*startobj != NULL) {
    if (Nj_ASGC(*startobj)->marked == UNMARKED) {
      NjObject* unmarked = *startobj;
      *startobj = ((NjVarObject*)unmarked)->next;
#if defined(Nj_DEBUG_OBJECT)
      fprintf(stdout, "NjObject<%p, '%s'> collected\n",
          unmarked, unmarked->ob_type->tp_name);
#endif
      njord_freeobj(unmarked, sizeof(GCHead));
      --vm->objcnt;
    }
    else {
      Nj_ASGC(*startobj)->marked = UNMARKED;
      startobj = &((NjVarObject*)*startobj)->next;
    }
  }
}

static void
njmarks_collect(NjObject* vm) {
  NjVMObject* _vm = (NjVMObject*)vm;
  int old_objcnt = _vm->objcnt;

  _njmarks_mark_all(_vm);
  _njmarks_sweep(_vm);

  if (_vm->maxobj < MAX_GC_THRESHOLD) {
    _vm->maxobj = _vm->objcnt << 1;
    if (_vm->maxobj > MAX_GC_THRESHOLD)
      _vm->maxobj = MAX_GC_THRESHOLD;
  }
  fprintf(stdout, "<%s> collected [%d] objects, [%d] remaining.\n",
      _vm->ob_type->tp_name, old_objcnt - _vm->objcnt, _vm->objcnt);
}

static NjObject*
njmarks_newvm(void) {
  NjVMObject* vm = (NjVMObject*)njmem_malloc(sizeof(NjVMObject));
  Nj_CHECK(vm != NULL, "create VM failed");

  vm->ob_type = &NjMarks_Type;
  vm->stackcnt = 0;
  vm->startobj = NULL;
  vm->objcnt = 0;
  vm->maxobj = INIT_GC_THRESHOLD;

  return (NjObject*)vm;
}

static void
njmarks_freevm(NjObject* vm) {
  njmarks_collect(vm);
  njmem_free(vm, sizeof(NjVMObject));
}

static NjObject*
njmarks_pushint(NjObject* vm, int value) {
  NjVMObject* _vm = (NjVMObject*)vm;
  if (_vm->objcnt >= _vm->maxobj)
    njmarks_collect(vm);

  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_ASGC(obj)->marked = UNMARKED;
  obj->next = _vm->startobj;
  _vm->startobj = (NjObject*)obj;
  ++_vm->objcnt;
  _njmarks_push(_vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static NjObject*
njmarks_pushpair(NjObject* vm) {
  NjVMObject* _vm = (NjVMObject*)vm;
  if (_vm->objcnt >= _vm->maxobj)
    njmarks_collect(vm);

  NjObject* tail = _njmarks_pop(_vm);
  NjObject* head = _njmarks_pop(_vm);
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, NULL, NULL);
  Nj_ASGC(obj)->marked = UNMARKED;
  obj->next = _vm->startobj;
  _vm->startobj = (NjObject*)obj;
  ++_vm->objcnt;
  _njmarks_push(_vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static void
njmarks_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (head != NULL)
    njord_pairsetter(pair, "head", head);

  if (tail != NULL)
    njord_pairsetter(pair, "tail", tail);
}

static void
njmarks_pop(NjObject* vm) {
  _njmarks_pop((NjVMObject*)vm);
}

static NjGCMethods gc_methods = {
  njmarks_newvm, /* gc_newvm */
  njmarks_freevm, /* gc_freevm */
  njmarks_pushint, /* gc_pushint */
  njmarks_pushpair, /* gc_pushpair */
  njmarks_setpair, /* gc_setpair */
  njmarks_pop, /* gc_pop */
  njmarks_collect, /* gc_collect */
};

NjTypeObject NjMarks_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "marks_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};
