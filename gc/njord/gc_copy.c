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
#include <stdlib.h> /* for malloc/free */
#include <string.h>
#include "njlog.h"
#include "njvm.h"

#define Nj_SEMISPACE_SIZE (512 << 10)
#define Nj_ASGC(ob)       ((GCHead*)(ob) - 1)
#define Nj_FROMGC(gc)     ((NjObject*)((GCHead*)(gc) + 1))
#define Nj_COPYSIZE(ob)   (((NjVarObject*)(ob))->ob_size + sizeof(GCHead))
#define Nj_FORWARDING(ob) (Nj_ASGC(ob)->forwarding)

static Nj_uchar_t* heaptr;
static Nj_uchar_t* fromspace;
static Nj_uchar_t* tospace;
static Nj_uchar_t* allocptr;

typedef struct _gc {
  NjObject* forwarding;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;
} NjVMObject;

static void
_copymem_init(void) {
  heaptr = (Nj_uchar_t*)malloc(2 * Nj_SEMISPACE_SIZE);
  Nj_CHECK(heaptr != NULL, "allocate copying space failed");

  tospace = heaptr;
  fromspace = tospace + Nj_SEMISPACE_SIZE;
  allocptr = tospace;
}

static void
_copymem_destroy(void* arg) {
  Nj_UNUSED(arg);
  free(heaptr);
}

static void*
_copymem_alloc(Nj_ssize_t n, void* arg) {
  NjObject* vm = (NjObject*)arg;
  if (allocptr + n > tospace + Nj_SEMISPACE_SIZE)
    Nj_GC(vm)->gc_collect(vm);
  Nj_CHECK(allocptr + n <= tospace + Nj_SEMISPACE_SIZE, "allocate failed");

  void* p = allocptr;
  allocptr += n;
  return p;
}

static NjObject*
_njcopy_copy(NjObject* vm, NjObject* obj) {
  if (Nj_FORWARDING(obj) == NULL) {
    NjObject* toref = Nj_FROMGC(allocptr);
    allocptr += Nj_COPYSIZE(obj);

    memmove(Nj_ASGC(toref), Nj_ASGC(obj), Nj_COPYSIZE(obj));
    Nj_FORWARDING(obj) = toref;

    if (obj->ob_type == &NjPair_Type) {
      NjObject* head = njord_pairgetter(toref, "head");
      if (head != NULL)
        njord_pairsetter(toref, "head", _njcopy_copy(vm, head));

      NjObject* tail = njord_pairgetter(toref, "tail");
      if (tail != NULL)
        njord_pairsetter(toref, "tail", _njcopy_copy(vm, tail));
    }
  }
  njlog_debug("<%s> copy NjObject<'%s'> [%p] to [%p]\n",
      vm->ob_type->tp_name, obj->ob_type->tp_name, obj, Nj_FORWARDING(obj));

  return Nj_FORWARDING(obj);
}

static void
njcopy_dealoc(NjObject* vm) {
  njvm_freevm(vm, (destroyvmfunc)_copymem_destroy);
}

static NjIntObject*
_njcopy_newint(NjObject* vm, Nj_int_t value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, _copymem_alloc, vm);
  Nj_FORWARDING(obj) = NULL;
  return obj;
}

static NjObject*
njcopy_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(vm, value, FALSE, _njcopy_newint);
}

static NjPairObject*
_njcopy_newpair(NjObject* vm, NjObject* head, NjObject* tail) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, _copymem_alloc, vm);
  Nj_FORWARDING(obj) = NULL;
  return obj;
}

static NjObject*
njcopy_pushpair(NjObject* vm) {
  return njvm_pushpair(vm, FALSE, _njcopy_newpair);
}

static void
njcopy_collect(NjObject* vm) {
  njlog_info("before flip <%s>: fromspace=%p, tospace=%p, allocptr=%p\n",
      vm->ob_type->tp_name, fromspace, tospace, allocptr);

  Nj_uchar_t* p = fromspace;
  fromspace = tospace;
  tospace = p;
  allocptr = tospace;

  njlog_info("after flip <%s>: fromspace=%p, tospace=%p, allocptr=%p\n",
      vm->ob_type->tp_name, fromspace, tospace, allocptr);
  for (int i = 0; i < Nj_VM(vm)->stackcnt; ++i)
    Nj_VM(vm)->stack[i] = _njcopy_copy(vm, Nj_VM(vm)->stack[i]);
}

static NjGCMethods gc_methods = {
  njcopy_dealoc, /* gc_dealloc */
  njcopy_pushint, /* gc_pushint */
  njcopy_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  0, /* gc_pop */
  njcopy_collect, /* gc_collect */
};

static NjTypeObject NjCopy_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "semispacescopy_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njcopy_vm_init(NjObject* vm) {
  ((NjVMObject*)vm)->ob_type = &NjCopy_Type;
  _copymem_init();
}

NjObject*
njsemispacecopy_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njcopy_vm_init);
}
