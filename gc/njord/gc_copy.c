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
#include <string.h>
#include "gc_impl.h"
#include "njmem.h"

#define MAX_STACK        (1024)
#define COPYGC_HALF_SIZE (512 << 10)
#define Nj_ASGC(ob)     ((GCHead*)(ob) - 1)
#define Nj_COPYN(ob)    (((NjVarObject*)(ob))->ob_size + sizeof(GCHead))

static Nj_uchar_t* copymem = NULL;
static Nj_uchar_t* fromspace = NULL;
static Nj_uchar_t* tospace = NULL;
static Nj_uchar_t* allocptr = NULL;

typedef struct _gc {
  NjObject* forward;
} GCHead;

typedef struct _vm {
  NjObject_HEAD;

  NjObject* stack[MAX_STACK];
  int stackcnt;
} NjVMObject;

static void njcopy_collect(NjObject* vm);

static void
_copymem_init(void) {
  copymem = (Nj_uchar_t*)malloc(2 * COPYGC_HALF_SIZE);
  Nj_CHECK(copymem != NULL, "allocate copying space failed");

  tospace = copymem;
  fromspace = tospace + COPYGC_HALF_SIZE;
  allocptr = tospace;
}

static void
_copymem_destroy(void) {
  free(copymem);
  copymem = NULL;
  fromspace = NULL;
  tospace = NULL;
  allocptr = NULL;
}

static void*
_copymem_alloc(NjVMObject* vm, Nj_ssize_t n) {
  if (allocptr + n > tospace + COPYGC_HALF_SIZE)
    njcopy_collect((NjObject*)vm);
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
  if (Nj_ASGC(obj)->forward == NULL) {
    Nj_uchar_t* p = allocptr;
    allocptr += Nj_COPYN(obj);

    memmove(p, Nj_ASGC(obj), Nj_COPYN(obj));
    NjObject* new_obj = (NjObject*)(p + sizeof(GCHead));
    Nj_ASGC(obj)->forward = new_obj;

    if (obj->ob_type == &NjPair_Type) {
      NjObject* head = njord_pairgetter(new_obj, "head");
      if (head != NULL)
        njord_pairsetter(new_obj, "head", _njcopy_copy(head));

      NjObject* tail = njord_pairgetter(new_obj, "tail");
      if (tail != NULL)
        njord_pairsetter(new_obj, "tail", _njcopy_copy(tail));
    }
  }
  fprintf(stdout, "<%s> copy NjObject<'%s'> [%p] to [%p]\n",
      NjCopy_Type.tp_name, obj->ob_type->tp_name, obj, Nj_ASGC(obj)->forward);

  return Nj_ASGC(obj)->forward;
}

static NjObject*
njcopy_newvm(void) {
  _copymem_init();
  NjVMObject* vm = (NjVMObject*)njmem_malloc(sizeof(NjVMObject));
  Nj_CHECK(vm != NULL, "create VM failed");

  vm->ob_type = &NjCopy_Type;
  vm->stackcnt = 0;

  return (NjObject*)vm;
}

static void
njcopy_freevm(NjObject* vm) {
  njcopy_collect(vm);
  njmem_free(vm, sizeof(NjVMObject));
  _copymem_destroy();
}

static NjObject*
njcopy_pushint(NjObject* vm, int value) {
  Nj_uchar_t* p = (Nj_uchar_t*)_copymem_alloc(
      (NjVMObject*)vm, sizeof(NjIntObject) + sizeof(GCHead));
  NjIntObject* obj = (NjIntObject*)(p + sizeof(GCHead));
  Nj_ASGC(obj)->forward = NULL;
  obj->ob_type = &NjInt_Type;
  obj->ob_size = sizeof(NjIntObject);
  obj->value = value;
  _njcopy_push((NjVMObject*)vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static NjObject*
njcopy_pushpair(NjObject* vm) {
  Nj_uchar_t* p = (Nj_uchar_t*)_copymem_alloc(
      (NjVMObject*)vm, sizeof(NjPairObject) + sizeof(GCHead));
  NjPairObject* obj = (NjPairObject*)(p + sizeof(GCHead));
  Nj_ASGC(obj)->forward = NULL;
  obj->ob_type = &NjPair_Type;
  obj->ob_size = sizeof(NjPairObject);
  obj->tail = _njcopy_pop((NjVMObject*)vm);
  obj->head = _njcopy_pop((NjVMObject*)vm);
  _njcopy_push((NjVMObject*)vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static void
njcopy_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (head != NULL)
    njord_pairsetter(pair, "head", head);

  if (tail != NULL)
    njord_pairsetter(pair, "tail", tail);
}

static void
njcopy_pop(NjObject* vm) {
  _njcopy_pop((NjVMObject*)vm);
}

static void
njcopy_collect(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  fprintf(stdout, "before flip <%s>: fromspace=%p, tospace=%p, allocptr=%p\n",
      vm->ob_type->tp_name, fromspace, tospace, allocptr);

  Nj_uchar_t* p = fromspace;
  fromspace = tospace;
  tospace = p;
  allocptr = tospace;

  fprintf(stdout, "after flip <%s>: fromspace=%p, tospace=%p, allocptr=%p\n",
      vm->ob_type->tp_name, fromspace, tospace, allocptr);
  for (int i = 0; i < vm->stackcnt; ++i)
    vm->stack[i] = _njcopy_copy(vm->stack[i]);
}

static NjGCMethods gc_methods = {
  njcopy_newvm, /* gc_newvm */
  njcopy_freevm, /* gc_freevm */
  njcopy_pushint, /* gc_pushint */
  njcopy_pushpair, /* gc_pushpair */
  njcopy_setpair, /* gc_setpair */
  njcopy_pop, /* gc_pop */
  njcopy_collect, /* gc_collect */
};

NjTypeObject NjCopy_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "copy_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};
