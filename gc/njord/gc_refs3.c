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
#include "njset.h"
#include "njvm.h"

#define Nj_ASGC(ob)   ((GCHead*)(ob) - 1)
#define Nj_REFCNT(ob) (Nj_ASGC(ob)->refcnt)
#define Nj_INCREF(ob) (++Nj_REFCNT(ob))
#define Nj_DECREF(ob) (--Nj_REFCNT(ob))

typedef struct _gc {
  Nj_ssize_t refcnt;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;

  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

static NjSet* zct; /* zero counting table */

static inline void
_njrefs_addref(NjObject* ref) {
  Nj_INCREF(ref);
}

static inline void
_njrefs_delref(NjObject* ref, NjObject* vm) {
  if (ref != NULL) {
    if (Nj_DECREF(ref) == 0) {
      if (Nj_ISPAIR(ref)) {
        _njrefs_delref(njord_pairgetter(ref, "head"), vm);
        _njrefs_delref(njord_pairgetter(ref, "tail"), vm);
      }
      njlog_debug("NjObject<%p, '%s'> collected\n",
          ref, ref->ob_type->tp_name);
      --Nj_VM(vm)->objcnt;
      njord_freeobj(ref, sizeof(GCHead));
    }
  }
}

static inline void
_njrefs_delref_tozct(NjObject* ref) {
  if (ref != NULL) {
    if (Nj_DECREF(ref) == 0)
      njset_add(zct, ref);
  }
}

static void
_njrefs_vm_destroy(NjObject* vm) {
  Nj_UNUSED(vm);
  njset_dealloc(zct);
}

static void
njrefs_dealloc(NjObject* vm) {
  njvm_freevm(vm, _njrefs_vm_destroy);
}

static NjIntObject*
_njrefs_newint(NjObject* vm, Nj_int_t value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_REFCNT(obj) = 0;
  ++Nj_VM(vm)->objcnt;
  njset_add(zct, Nj_ASOBJ(obj));
  return obj;
}

static NjObject*
njrefs_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(vm,
      value, Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njrefs_newint);
}

static NjPairObject*
_njrefs_newpair(NjObject* vm) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(sizeof(GCHead), NULL, NULL);
  Nj_REFCNT(obj) = 0;
  ++Nj_VM(vm)->objcnt;
  njset_add(zct, Nj_ASOBJ(obj));
  return obj;
}

static void
_njrefs_initpair(NjObject* vm,
    NjPairObject* obj, NjObject* head, NjObject* tail) {
  Nj_UNUSED(vm), Nj_UNUSED(obj);
  _njrefs_addref(head);
  _njrefs_addref(tail);
  njset_remove(zct, head);
  njset_remove(zct, tail);
}

static NjObject*
njrefs_pushpair(NjObject* vm) {
  return njvm_pushpair(vm, Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj,
      _njrefs_newpair, _njrefs_initpair);
}

static void
njrefs_setpair(NjObject* obj, NjObject* head, NjObject* tail) {
  if (head != NULL) {
    _njrefs_addref(head);
    njset_remove(zct, head);
    _njrefs_delref_tozct(njord_pairgetter(obj, "head"));
    njord_pairsetter(obj, "head", head);
  }

  if (tail != NULL) {
    _njrefs_addref(tail);
    njset_remove(zct, tail);
    _njrefs_delref_tozct(njord_pairgetter(obj, "tail"));
    njord_pairsetter(obj, "tail", tail);
  }
}

static void
_njrefs_sweep_visist(NjObject* obj, void* arg) {
  NjObject* vm = (NjObject*)arg;
  if (Nj_REFCNT(obj) == 0) {
    if (Nj_ISPAIR(obj)) {
      _njrefs_delref(njord_pairgetter(obj, "head"), vm);
      _njrefs_delref(njord_pairgetter(obj, "tail"), vm);
    }
    --Nj_VM(vm)->objcnt;
    njord_freeobj(obj, sizeof(GCHead));
  }
}

static void
njrefs_collect(NjObject* vm) {
  for (int i = 0; i < Nj_VM(vm)->stackcnt; ++i)
    _njrefs_addref(Nj_VM(vm)->stack[i]);

  Nj_int_t old_objcnt = Nj_VM(vm)->objcnt;
  /* sweep zct */
  njset_traverse(zct, _njrefs_sweep_visist, vm);
  njset_clear(zct);

  for (int i = 0; i < Nj_VM(vm)->stackcnt; ++i)
    _njrefs_delref_tozct(Nj_VM(vm)->stack[i]);

  if (Nj_VM(vm)->maxobj < Nj_GC_MAXTHRESHOLD) {
    Nj_VM(vm)->maxobj = Nj_VM(vm)->objcnt << 1;
    if (Nj_VM(vm)->maxobj > Nj_GC_MAXTHRESHOLD)
      Nj_VM(vm)->maxobj = Nj_GC_MAXTHRESHOLD;
  }
  njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
      Nj_VM(vm)->ob_type->tp_name,
      old_objcnt - Nj_VM(vm)->objcnt, Nj_VM(vm)->objcnt);
}

static NjGCMethods gc_methods = {
  njrefs_dealloc, /* gc_dealloc */
  njrefs_pushint, /* gc_pushint */
  njrefs_pushpair, /* gc_pushpair */
  njrefs_setpair, /* gc_setpair */
  0, /* gc_pop */
  njrefs_collect, /* gc_collect */
};

static NjTypeObject NjDeferRefs_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "deferrefs_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njrefs_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjDeferRefs_Type;
  Nj_VM(vm)->objcnt = 0;
  Nj_VM(vm)->maxobj = Nj_GC_INITTHRESHOLD;
  zct = njset_create();
}

NjObject*
njrefs3_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njrefs_vm_init);
}
