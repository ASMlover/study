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
#define INIT_GC_THRESHOLD (8)

typedef enum _marked {
  UNMARKED,
  MARKED,
} NjMarked;

typedef struct _varobject {
  NjObject_HEAD

  Nj_uchar_t marked;
  struct _varobject* next;
  NjVarType type;
  union {
    int value;
    struct {
      struct _varobject* head;
      struct _varobject* tail;
    };
  };
} NjMarkSObject;

typedef struct _vm {
  NjObject_HEAD

  NjMarkSObject* stack[MAX_STACK];
  int stackcnt;

  NjMarkSObject* startobj;
  int objcnt;
  int maxobj;
} NjMarkSVM;

static void
_njord_push(NjMarkSVM* vm, NjMarkSObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjMarkSObject*
_njord_pop(NjMarkSVM* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static void
_njord_mark(NjMarkSObject* obj) {
  if (obj->marked == MARKED)
    return;

  obj->marked = MARKED;
  if (obj->type == VAR_PAIR) {
    _njord_mark(obj->head);
    _njord_mark(obj->tail);
  }
}

static void
_njord_mark_all(NjMarkSVM* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njord_mark(vm->stack[i]);
}

static void
_njord_sweep(NjMarkSVM* vm) {
  NjMarkSObject** startobj = &vm->startobj;
  while (*startobj != NULL) {
    if ((*startobj)->marked == UNMARKED) {
      NjMarkSObject* unmarked = *startobj;
      startobj = &unmarked->next;
      njmem_free(unmarked, sizeof(NjMarkSObject));
      --vm->objcnt;
    }
    else {
      (*startobj)->marked = UNMARKED;
      startobj = &(*startobj)->next;
    }
  }
}

static void
njmarks_collect(NjObject* vm) {
  NjMarkSVM* _vm = (NjMarkSVM*)vm;
  int old_objcnt = _vm->objcnt;

  _njord_mark_all(_vm);
  _njord_sweep(_vm);

  _vm->maxobj = (int)(old_objcnt * 1.5);
  fprintf(stdout, "NjGC_MarkS collected [%d] objects, [%d] remaining.\n",
      old_objcnt - _vm->objcnt, _vm->objcnt);
}

static NjMarkSObject*
_njord_new_object(NjMarkSVM* vm, NjVarType type) {
  if (vm->objcnt >= vm->maxobj)
    njmarks_collect((NjObject*)vm);

  NjMarkSObject* obj = (NjMarkSObject*)njmem_malloc(sizeof(NjMarkSObject));
  obj->ob_name = "marks_object";
  obj->marked = UNMARKED;
  obj->type = type;
  obj->next = vm->startobj;
  vm->startobj = obj;
  ++vm->objcnt;

  return obj;
}

static NjObject*
njmarks_new(void) {
  NjMarkSVM* vm = (NjMarkSVM*)njmem_malloc(sizeof(NjMarkSVM));
  Nj_CHECK(vm != NULL, "create VM failed");

  vm->ob_name = "NjMarkSVM";
  vm->stackcnt = 0;
  vm->startobj = NULL;
  vm->objcnt = 0;
  vm->maxobj = INIT_GC_THRESHOLD;

  return (NjObject*)vm;
}

static void
njmarks_free(NjObject* vm) {
  njmarks_collect(vm);
  njmem_free(vm, sizeof(NjMarkSVM));
}

static NjObject*
njmarks_pushint(NjObject* vm, int value) {
  NjMarkSObject* obj = _njord_new_object((NjMarkSVM*)vm, VAR_INT);
  obj->value = value;
  _njord_push((NjMarkSVM*)vm, obj);

  return (NjObject*)obj;
}

static NjObject*
njmarks_pushpair(NjObject* vm) {
  NjMarkSObject* obj = _njord_new_object((NjMarkSVM*)vm, VAR_PAIR);
  obj->tail = _njord_pop((NjMarkSVM*)vm);
  obj->head = _njord_pop((NjMarkSVM*)vm);
  _njord_push((NjMarkSVM*)vm, obj);

  return (NjObject*)obj;
}

static void
njmarks_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (head != NULL)
    ((NjMarkSObject*)pair)->head = (NjMarkSObject*)head;

  if (tail != NULL)
    ((NjMarkSObject*)pair)->tail = (NjMarkSObject*)tail;
}

static void
njmarks_pop(NjObject* vm) {
  _njord_pop((NjMarkSVM*)vm);
}

static NjGCMethods marks_methods = {
  njmarks_new, /* tp_newvm */
  njmarks_free, /* tp_freevm */
  njmarks_pushint, /* tp_pushint */
  njmarks_pushpair, /* tp_pushpair */
  njmarks_setpair, /* tp_setpair */
  njmarks_pop, /* tp_pop */
  njmarks_collect, /* tp_collect */
};

NjGCObject NjGC_MarkS = {
  GC_MARK_SWEEP, /* type */
  (NjGCMethods*)&marks_methods, /* methods */
};
