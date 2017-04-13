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
#include "njmem.h"
#include "gc_impl.h"

#define MAX_STACK (1024)

#define Nj_ASGC(ob)   ((GCHead*)(ob) - 1)
#define Nj_REFCNT(ob) (Nj_ASGC(ob)->refcnt)
#define Nj_NEWREF(ob) (Nj_REFCNT(ob) = 1)
#define Nj_INCREF(ob) (++Nj_ASGC(ob)->refcnt)
#define Nj_DECREF(ob) do {\
  if (--Nj_ASGC(ob)->refcnt == 0) {\
    fprintf(stdout, "NjObject<%p, '%s'> collected\n",\
        ((NjObject*)(ob)),\
        ((NjObject*)(ob))->ob_type->tp_name);\
    _njrefs_dealloc(ob);\
  }\
} while (0)
#define Nj_XINCREF(ob) do { if ((ob) != NULL) Nj_INCREF(ob); } while (0)
#define Nj_XDECREF(ob) do { if ((ob) != NULL) Nj_DECREF(ob); } while (0)

typedef struct _gc {
  Nj_ssize_t refcnt;
} GCHead;

typedef struct _vm {
  NjObject_HEAD;

  NjObject* stack[MAX_STACK];
  int stackcnt;
} NjVMObject;

static void
_njrefs_push(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njrefs_pop(NjVMObject* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static void
_njrefs_dealloc(NjObject* obj) {
  if (obj->ob_type == &NjPair_Type) {
    NjObject* head = njord_pairgetter(obj, "head");
    Nj_XDECREF(head);
    NjObject* tail = njord_pairgetter(obj, "tail");
    Nj_XDECREF(tail);
  }
  njord_freeobj(obj, sizeof(GCHead));
}

static void njrefs_pop(NjObject* vm);

static NjObject*
njrefs_newvm(void) {
  NjVMObject* vm = (NjVMObject*)njmem_malloc(sizeof(NjVMObject));
  Nj_CHECK(vm != NULL, "create VM failed");

  vm->ob_type = &NjRefs_Type;
  vm->stackcnt = 0;

  return (NjObject*)vm;
}

static void
njrefs_freevm(NjObject* vm) {
  while (((NjVMObject*)vm)->stackcnt > 0)
    njrefs_pop(vm);
  njmem_free(vm, sizeof(NjVMObject));
}

static NjObject*
njrefs_pushint(NjObject* vm, int value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_NEWREF(obj);
  _njrefs_push((NjVMObject*)vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static NjObject*
njrefs_pushpair(NjObject* vm) {
  NjObject* tail = _njrefs_pop((NjVMObject*)vm);
  NjObject* head = _njrefs_pop((NjVMObject*)vm);

  Nj_INCREF(head);
  Nj_INCREF(tail);
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, NULL, NULL);
  Nj_NEWREF(obj);
  Nj_DECREF(head);
  Nj_DECREF(tail);
  _njrefs_push((NjVMObject*)vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static void
njrefs_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (head != NULL) {
    NjObject* old_head = njord_pairgetter(pair, "head");
    Nj_DECREF(old_head);
    Nj_INCREF(head);
    njord_pairsetter(pair, "head", head);
  }

  if (tail != NULL) {
    NjObject* old_tail = njord_pairgetter(pair, "tail");
    Nj_DECREF(old_tail);
    Nj_INCREF(tail);
    njord_pairsetter(pair, "tail", tail);
  }
}

static void
njrefs_pop(NjObject* vm) {
  NjObject* obj = _njrefs_pop((NjVMObject*)vm);
  Nj_DECREF(obj);
}

static NjGCMethods gc_methods = {
  njrefs_newvm, /* gc_newvm */
  njrefs_freevm, /* gc_freevm */
  njrefs_pushint, /* gc_pushint */
  njrefs_pushpair, /* gc_pushpair */
  njrefs_setpair, /* gc_setpair */
  njrefs_pop, /* gc_pop */
  0, /* gc_collect */
};

NjTypeObject NjRefs_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "refs_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};
