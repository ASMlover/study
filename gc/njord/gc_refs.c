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
#include "njlog.h"
#include "njmem.h"
#include "njvm.h"

#define Nj_ASGC(ob)   ((GCHead*)(ob) - 1)
#define Nj_REFCNT(ob) (Nj_ASGC(ob)->refcnt)
#define Nj_NEWREF(ob) (Nj_REFCNT(ob) = 1)
#define Nj_INCREF(ob) (++Nj_ASGC(ob)->refcnt)
#define Nj_DECREF(ob) do {\
  if (--Nj_ASGC(ob)->refcnt == 0) {\
    njlog_debug("NjObject<%p, '%s'> collected\n",\
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
  NjObject_VM_HEAD;
} NjVMObject;

static void
_njrefs_push(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < Nj_VMSTACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njrefs_pop(NjVMObject* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static void
_njrefs_dealloc(NjObject* obj) {
  if (Nj_ISPAIR(obj)) {
    NjObject* head = njord_pairgetter(obj, "head");
    Nj_XDECREF(head);
    NjObject* tail = njord_pairgetter(obj, "tail");
    Nj_XDECREF(tail);
  }
  njord_freeobj(obj, sizeof(GCHead));
}

static NjObject*
njrefs_pushint(NjObject* vm, Nj_int_t value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_NEWREF(obj);
  _njrefs_push((NjVMObject*)vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static NjObject*
njrefs_pushpair(NjObject* vm) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(sizeof(GCHead), NULL, NULL);
  NjObject* tail = _njrefs_pop((NjVMObject*)vm);
  NjObject* head = _njrefs_pop((NjVMObject*)vm);
  Nj_INCREF(head);
  Nj_INCREF(tail);
  obj->head = head;
  obj->tail = tail;
  Nj_DECREF(head);
  Nj_DECREF(tail);
  Nj_NEWREF(obj);
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

static void
njrefs_collect(NjObject* vm) {
  while (Nj_VM(vm)->stackcnt > 0)
    Nj_GC(vm)->gc_pop(vm);
}

static NjGCMethods gc_methods = {
  0, /* gc_dealloc */
  njrefs_pushint, /* gc_pushint */
  njrefs_pushpair, /* gc_pushpair */
  njrefs_setpair, /* gc_setpair */
  njrefs_pop, /* gc_pop */
  njrefs_collect, /* gc_collect */
};

static NjTypeObject NjRefs_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "refs_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njrefs_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjRefs_Type;
}

NjObject*
njrefs_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njrefs_vm_init);
}
