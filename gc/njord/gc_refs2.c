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
#define Nj_INCREF(ob) (++Nj_REFCNT(ob))
#define Nj_DECREF(ob) (--Nj_REFCNT(ob))

typedef struct _gc {
  Nj_ssize_t refcnt;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;
} NjVMObject;

static inline void
_njrefs_addref(NjObject* ref) {
  if (ref != NULL)
    Nj_INCREF(ref);
}

static inline void
_njrefs_delref(NjObject* ref) {
  if (ref != NULL) {
    Nj_DECREF(ref);
    if (Nj_REFCNT(ref) == 0) {
      if (Nj_ISPAIR(ref)) {
        NjObject* head = njord_pairgetter(ref, "head");
        _njrefs_delref(head);
        NjObject* tail = njord_pairgetter(ref, "tail");
        _njrefs_delref(tail);
      }
      njlog_debug("NjObject<%p, '%s'> collected\n",
          ref,
          ref->ob_type->tp_name);
      njord_freeobj(ref, sizeof(GCHead));
    }
  }
}

static NjIntObject*
_njrefs_newint(NjObject* vm, Nj_int_t value) {
  Nj_UNUSED(vm);
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_NEWREF(obj);
  return obj;
}

static NjObject*
njrefs_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(vm, value, FALSE, _njrefs_newint);
}

static NjPairObject*
_njrefs_newpair(NjObject* vm, NjObject* head, NjObject* tail) {
  Nj_UNUSED(vm);

  _njrefs_addref(head);
  _njrefs_addref(tail);
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, NULL, NULL);
  Nj_NEWREF(obj);
  _njrefs_delref(head);
  _njrefs_delref(tail);
  return obj;
}

static NjObject*
njrefs_pushpair(NjObject* vm) {
  return njvm_pushpair(vm, FALSE, _njrefs_newpair);
}

static void
njrefs_setpair(NjObject* obj, NjObject* head, NjObject* tail) {
  if (head != NULL) {
    _njrefs_addref(head);
    _njrefs_delref(njord_pairgetter(obj, "head"));
    njord_pairsetter(obj, "head", head);
  }

  if (tail != NULL) {
    _njrefs_addref(tail);
    _njrefs_delref(njord_pairgetter(obj, "tail"));
    njord_pairsetter(obj, "tail", tail);
  }
}

static void
njrefs_pop(NjObject* vm) {
  _njrefs_delref(njvm_pop(vm));
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

static NjTypeObject NjRefs2_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "refs2_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_repr */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njrefs_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjRefs2_Type;
}

NjObject*
njrefs2_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njrefs_vm_init);
}
