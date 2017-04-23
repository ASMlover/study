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
#include "njmem.h"
#include "njlog.h"
#include "njvm.h"

#define Nj_SEMISPACE_SIZE (512 << 10)
#define Nj_ASGC(ob)       ((GCHead*)(ob) - 1)
#define Nj_FROMGC(gc)     ((NjObject*)((GCHead*)(gc) + 1))
#define Nj_COPYSIZE(ob)   (((NjVarObject*)(ob))->ob_size + sizeof(GCHead))
#define Nj_FORWARDING(ob) (Nj_ASGC(ob)->forwarding)

static Nj_uchar_t* heaptr; /* heap start of semispace gc */
static Nj_uchar_t* fromspace;
static Nj_uchar_t* tospace;
static Nj_uchar_t* allocptr;
static Nj_uchar_t* scanptr;

static void
_njsemispace_init(void) {
  heaptr = (Nj_uchar_t*)malloc(2 * Nj_SEMISPACE_SIZE);
  Nj_CHECK(heaptr != NULL, "create semispace failed");

  fromspace = heaptr + Nj_SEMISPACE_SIZE;
  tospace = heaptr;
  allocptr = tospace;
}

static void
_njsemispace_destroy(void* arg) {
  Nj_UNUSED(arg);
  free(heaptr);
}

static void*
_njsemispace_alloc(Nj_ssize_t n, void* arg) {
  NjObject* vm = (NjObject*)arg;
  if (allocptr + n > tospace + Nj_SEMISPACE_SIZE)
    Nj_GC(vm)->gc_collect(vm);
  Nj_CHECK(allocptr + n <= tospace + Nj_SEMISPACE_SIZE, "allocating failed");

  void* p = allocptr;
  allocptr += n;
  return p;
}

typedef struct _gc {
  NjObject* forwarding;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;
  Nj_int_t objcnt;
} NjVMObject;

static void
njsemispacecopy_dealloc(NjObject* vm) {
  njvm_freevm(vm, (destroyvmfunc)_njsemispace_destroy);
}

static NjIntObject*
_njsemispacecopy_newint(NjObject* vm, int value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, _njsemispace_alloc, vm);
  Nj_FORWARDING(obj) = NULL;
  ++((NjVMObject*)vm)->objcnt;
  return obj;
}

static NjObject*
njsemispacecopy_pushint(NjObject* vm, int value) {
  return njvm_pushint(vm, value, FALSE, _njsemispacecopy_newint);
}

static NjPairObject*
_njsemispacecopy_newpair(NjObject* vm, NjObject* head, NjObject* tail) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, _njsemispace_alloc, vm);
  Nj_FORWARDING(obj) = NULL;
  ++((NjVMObject*)vm)->objcnt;
  return obj;
}

static NjObject*
njsemispacecopy_pushpair(NjObject* vm) {
  return njvm_pushpair(vm, FALSE, _njsemispacecopy_newpair);
}

#define _worklist_init()  (scanptr = allocptr)
#define _worklist_empty() (scanptr == allocptr)
#define _worklist_add(ob) Nj_UNUSED(ob)

static NjObject*
_worklist_remove(void) {
  NjObject* ref = Nj_FROMGC(scanptr);
  scanptr += Nj_COPYSIZE(ref);
  return ref;
}

static NjObject*
_njsemispacecopy_copy(NjObject* fromref) {
  NjObject* toref = Nj_FROMGC(allocptr);
  allocptr += Nj_COPYSIZE(fromref);

  memmove(Nj_ASGC(toref), Nj_ASGC(fromref), Nj_COPYSIZE(fromref));
  Nj_FORWARDING(fromref) = toref;
  _worklist_add(toref);

  return toref;
}

static NjObject*
_njsemispacecopy_forward(NjVMObject* vm, NjObject* fromref) {
  NjObject* toref = Nj_FORWARDING(fromref);
  if (toref == NULL) {
    toref = _njsemispacecopy_copy(fromref);
    ++vm->objcnt;
  }
  return toref;
}

static void
njsemispacecopy_collect(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  Nj_int_t old_objcnt = vm->objcnt;
  njlog_info("<%s> before collect: fromspace=%p, tospace=%p, allocptr=%p\n",
      vm->ob_type->tp_name, fromspace, tospace, allocptr);

  /* switch semispaces */
  Nj_uchar_t* tmp = fromspace;
  fromspace = tospace;
  tospace = tmp;
  allocptr = tospace;

  /* copy the roots */
  vm->objcnt = 0;
  _worklist_init();
  for (int i = 0; i < vm->stackcnt; ++i)
    vm->stack[i] = _njsemispacecopy_forward(vm, vm->stack[i]);
  /* copy transitive closure */
  while (!_worklist_empty()) {
    NjObject* ref = _worklist_remove();
    if (ref->ob_type == &NjPair_Type) {
      NjObject* head = njord_pairgetter(ref, "head");
      if (head != NULL)
        njord_pairsetter(ref, "head", _njsemispacecopy_forward(vm, head));

      NjObject* tail = njord_pairgetter(ref, "tail");
      if (tail != NULL)
        njord_pairsetter(ref, "tail", _njsemispacecopy_forward(vm, tail));
    }
  }

  njlog_info("<%s> after collect: fromspace=%p, tospace=%p, allocptr=%p, "
      "collected [%d] objects, [%d] remaining\n",
      vm->ob_type->tp_name, fromspace, tospace, allocptr,
      old_objcnt - vm->objcnt, vm->objcnt);
}

static NjGCMethods gc_methods = {
  njsemispacecopy_dealloc, /* gc_dealloc */
  njsemispacecopy_pushint, /* gc_pushint */
  njsemispacecopy_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  0, /* gc_pop */
  njsemispacecopy_collect, /* gc_collect */
};

NjTypeObject NjCopy2_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "semispacescopy2_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njsemispacecopy_vm_init(NjObject* vm) {
  ((NjVMObject*)vm)->ob_type = &NjCopy2_Type;
  ((NjVMObject*)vm)->objcnt = 0;
  _njsemispace_init();
}

NjObject*
njsemispacecopy2_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njsemispacecopy_vm_init);
}
