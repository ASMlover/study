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
#include <stdlib.h>
#include <string.h>
#include "njlog.h"
#include "njvm.h"

#define Nj_COMPACTION_SIZE  (512 << 10)
#define Nj_ASGC(ob)         ((GCHead*)(ob) - 1)
#define Nj_FORWARDING(ob)\
  ((NjObject*)(Nj_ASGC(ob)->forwarding + sizeof(GCHead)))

static Nj_uchar_t* heaptr;
static Nj_uchar_t* allocptr;

static void
_njcompactheap_init(void) {
  heaptr = (Nj_uchar_t*)malloc(Nj_COMPACTION_SIZE);
  Nj_CHECK(heaptr != NULL, "allocating heap failed");

  allocptr = heaptr;
}

static void
_njcompactheap_destroy(void* arg) {
  Nj_UNUSED(arg);
  free(heaptr);
}

static void*
_njcompactheap_alloc(Nj_ssize_t n, void* arg) {
  NjObject* vm = (NjObject*)arg;

  void* p = NULL;
  if (allocptr + n > heaptr + Nj_COMPACTION_SIZE)
    Nj_GC(vm)->gc_collect(vm);

  if (allocptr + n <= heaptr + Nj_COMPACTION_SIZE) {
    p = allocptr;
    allocptr += n;
  }
  Nj_CHECK(p != NULL, "allocating object memory failed");

  return p;
}

typedef struct _gc {
  Nj_uchar_t* forwarding;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;

  NjObject* startobj;
  NjObject* endobj;
  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

static void
_njcompact_insert(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK((Nj_uchar_t*)obj > (Nj_uchar_t*)vm->endobj,
      "allocating new object failed");

  ((NjVarObject*)obj)->next = NULL;
  if (vm->startobj == NULL) {
    vm->startobj = vm->endobj = obj;
  }
  else {
    ((NjVarObject*)vm->endobj)->next = obj;
    vm->endobj = obj;
  }
}

static void
_njcompact_mark(NjObject* obj) {
  if (njmark_ismarked(obj))
    return;

  njmark_setmark(obj);
  if (obj->ob_type == &NjPair_Type) {
    NjObject* head = njord_pairgetter(obj, "head");
    if (head != NULL && !njmark_ismarked(head))
      _njcompact_mark(head);

    NjObject* tail = njord_pairgetter(obj, "tail");
    if (tail != NULL && !njmark_ismarked(tail))
      _njcompact_mark(tail);
  }
}

static void
_njcompact_mark_all(NjVMObject* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njcompact_mark(vm->stack[i]);
}

static void
_njcompact_sweep(NjVMObject* vm) {
  Nj_uchar_t* freeptr = heaptr;
  NjObject* scan = vm->startobj;

  /* setting new forwarding address */
  while (scan != NULL) {
    if (njmark_ismarked(scan)) {
      Nj_ssize_t size = ((NjVarObject*)scan)->ob_size + sizeof(GCHead);
      Nj_ASGC(scan)->forwarding = freeptr;
      freeptr += size;
    }
    scan = ((NjVarObject*)scan)->next;
  }

  /* updated roots */
  for (int i = 0; i < vm->stackcnt; ++i)
    vm->stack[i] = Nj_FORWARDING(vm->stack[i]);

  /* updated fields */
  scan = vm->startobj;
  while (scan != NULL) {
    if (njmark_ismarked(scan)) {
      if (scan->ob_type == &NjPair_Type) {
        NjObject* head = njord_pairgetter(scan, "head");
        if (head != NULL)
          njord_pairsetter(scan, "head", Nj_FORWARDING(head));

        NjObject* tail = njord_pairgetter(scan, "tail");
        if (tail != NULL)
          njord_pairsetter(scan, "tail", Nj_FORWARDING(tail));
      }
    }
    scan = ((NjVarObject*)scan)->next;
  }

  /* relocating the object address */
  NjObject* next = NULL;
  scan = vm->startobj;
  vm->startobj = vm->endobj = NULL;
  while (scan != NULL) {
    next = ((NjVarObject*)scan)->next;
    if (njmark_ismarked(scan)) {
      Nj_ssize_t size = ((NjVarObject*)scan)->ob_size + sizeof(GCHead);
      Nj_uchar_t* forwarding = Nj_ASGC(scan)->forwarding;
      NjObject* forwarding_obj = Nj_FORWARDING(scan);

      memmove(forwarding, Nj_ASGC(scan), size);
      ((NjVarObject*)forwarding_obj)->next = NULL;
      if (vm->startobj == NULL) {
        vm->startobj = vm->endobj = forwarding_obj;
      }
      else {
        ((NjVarObject*)vm->endobj)->next = forwarding_obj;
        vm->endobj = forwarding_obj;
      }
    }
    else {
      --vm->objcnt;
    }
    scan = next;
  }
  allocptr = freeptr;
  njmark_init();
}

static void
njcompact_dealloc(NjObject* vm) {
  njvm_freevm(vm, (destroyvmfunc)_njcompactheap_destroy);
}

static NjIntObject*
_njcompact_newint(NjObject* _vm, int value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, _njcompactheap_alloc, vm);
  _njcompact_insert(vm, (NjObject*)obj);
  ++vm->objcnt;
  return obj;
}

static NjObject*
njcompact_pushint(NjObject* _vm, int value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  return njvm_pushint(_vm, value, vm->objcnt >= vm->maxobj, _njcompact_newint);
}

static NjPairObject*
_njcompact_newpair(NjObject* _vm, NjObject* head, NjObject* tail) {
  NjVMObject* vm = (NjVMObject*)_vm;
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, _njcompactheap_alloc, vm);
  _njcompact_insert(vm, (NjObject*)obj);
  ++vm->objcnt;
  return obj;
}

static NjObject*
njcompact_pushpair(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  return njvm_pushpair(_vm, vm->objcnt >= vm->maxobj, _njcompact_newpair);
}

static void
njcompact_collect(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  Nj_int_t old_objcnt = vm->objcnt;

  _njcompact_mark_all(vm);
  _njcompact_sweep(vm);

  if (vm->maxobj < Nj_GC_MAXTHRESHOLD) {
    vm->maxobj = vm->objcnt << 1;
    if (vm->maxobj > Nj_GC_MAXTHRESHOLD)
      vm->maxobj = Nj_GC_MAXTHRESHOLD;
  }

  njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
      vm->ob_type->tp_name, old_objcnt - vm->objcnt, vm->objcnt);
}

static NjGCMethods gc_methods = {
  njcompact_dealloc, /* gc_dealloc */
  njcompact_pushint, /* gc_pushint */
  njcompact_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  0, /* gc_pop */
  njcompact_collect, /* gc_collect */
};

static NjTypeObject NjCompaction_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "markcompaction_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_repr */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njcompact_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjCompaction_Type;
  Nj_VM(vm)->startobj = NULL;
  Nj_VM(vm)->endobj = NULL;
  Nj_VM(vm)->objcnt = 0;
  Nj_VM(vm)->maxobj = Nj_GC_INITTHRESHOLD;
  _njcompactheap_init();
}

NjObject*
njcompact_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njcompact_vm_init);
}
