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
#include "njlog.h"
#include "njvm.h"

#define INIT_GC_THRESHOLD (64)
#define MAX_GC_THRESHOLD  (1024)
#define LAZY_HEAP_SIZE    (512 << 10)
#define ALIGNMENT         (8)
#define ROUND_UP(n)       (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

typedef struct _block {
  struct _block* nextblock;
  Nj_ssize_t blocksz;
} Block;

static Nj_uchar_t* heap_address = NULL;
static Nj_uchar_t* allocptr = NULL;
static Block* freeblock = NULL;

static void
_njlazyheap_init(void) {
  heap_address = (Nj_uchar_t*)malloc(LAZY_HEAP_SIZE);
  Nj_CHECK(heap_address != NULL, "init lazy heap failed");

  allocptr = heap_address;
  freeblock = NULL;
}

static void
_njlazyheap_destroy(void* arg) {
  Nj_UNUSED(arg);
  free(heap_address);

  heap_address = NULL;
  allocptr = NULL;
}

static void*
_njlazyheap_alloc(Nj_ssize_t bytes) {
  void* p = NULL;
  if (allocptr + bytes <= heap_address + LAZY_HEAP_SIZE) {
    p = allocptr;
    allocptr += bytes;
  }

  return p;
}

typedef struct _vm {
  NjObject_VM_HEAD;

  NjObject* startobj;
  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

static NjVarObject* reclaimlist;

static void
_njlazysweep_mark(NjObject* obj) {
  if (njmark_ismarked(obj))
    return;

  njmark_setmark(obj);
  if (obj->ob_type == &NjPair_Type) {
    NjObject* head = njord_pairgetter(obj, "head");
    if (head != NULL && !njmark_ismarked(head))
      _njlazysweep_mark(head);

    NjObject* tail = njord_pairgetter(obj, "tail");
    if (tail != NULL && !njmark_ismarked(tail))
      _njlazysweep_mark(tail);
  }
}

static void
_njlazysweep_mark_all(NjVMObject* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njlazysweep_mark(vm->stack[i]);

  NjObject** startobj = &vm->startobj;
  while (*startobj != NULL) {
    if (!njmark_ismarked(*startobj)) {
      NjVarObject* unmarked = (NjVarObject*)*startobj;
      *startobj = unmarked->next;
      unmarked->next = (NjObject*)reclaimlist;
      reclaimlist = unmarked;
    }
    else {
      njmark_unsetmark(*startobj);
      startobj = &((NjVarObject*)*startobj)->next;
    }
  }
}

static void*
_njlazysweep_alloc(Nj_ssize_t n) {
  void* p = NULL;
  if (freeblock != NULL) {
    Block** block = &freeblock;
    while (*block != NULL && p == NULL) {
      if ((*block)->blocksz >= n) {
        p = *block;
        *block = (*block)->nextblock;
      }
      else {
        block = &(*block)->nextblock;
      }
    }

    if (p == NULL)
      p = _njlazyheap_alloc(n);
  }
  else {
    p = _njlazyheap_alloc(n);
  }

  return p;
}

static void*
_njlazysweep_lazy_sweep(NjVMObject* vm, Nj_ssize_t n) {
  Nj_int_t old_objcnt = vm->objcnt;
  while (reclaimlist != NULL) {
    Nj_ssize_t size = ROUND_UP(reclaimlist->ob_size);
    Block* block = (Block*)reclaimlist;
    reclaimlist = (NjVarObject*)reclaimlist->next;
    --vm->objcnt;
    if (size >= n) {
      njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
          vm->ob_type->tp_name, old_objcnt - vm->objcnt, vm->objcnt);
      return block;
    }

    if ((Nj_uchar_t*)block + size == allocptr) {
      allocptr = (Nj_uchar_t*)block;
    }
    else {
      block->nextblock = freeblock;
      block->blocksz = size;
      freeblock = block;
    }
  }

  njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
      vm->ob_type->tp_name, old_objcnt - vm->objcnt, vm->objcnt);
  return _njlazysweep_alloc(n);
}

static void*
_njlazysweep_newobj(Nj_ssize_t n, void* arg) {
  NjVMObject* vm = (NjVMObject*)arg;
  n = ROUND_UP(n);
  void* p = _njlazysweep_alloc(n);
  if (p == NULL) {
    if (reclaimlist != NULL)
      p = _njlazysweep_lazy_sweep(vm, n);

    if (p == NULL) {
      _njlazysweep_mark_all(vm);
      p = _njlazysweep_lazy_sweep(vm, n);
      Nj_CHECK(p != NULL, "allocate free memory space failed");
    }
  }

  return p;
}

static void
njlazysweep_collect(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;

  _njlazysweep_mark_all(vm);
  _njlazysweep_lazy_sweep(vm, LAZY_HEAP_SIZE);

  if (vm->maxobj < MAX_GC_THRESHOLD) {
    vm->maxobj = vm->objcnt << 1;
    if (vm->maxobj > MAX_GC_THRESHOLD)
      vm->maxobj = MAX_GC_THRESHOLD;
  }
}

static void
_njlazysweep_vm_init(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  vm->ob_type = &NjLazy_Type;
  vm->startobj = NULL;
  vm->objcnt = 0;
  vm->maxobj = INIT_GC_THRESHOLD;
  _njlazyheap_init();
}

static NjObject*
njlazysweep_newvm(void) {
  return njvm_newvm(sizeof(NjVMObject), _njlazysweep_vm_init);
}

static void
njlazysweep_freevm(NjObject* vm) {
  njvm_freevm(vm, (destroyvmfunc)_njlazyheap_destroy);
}

static NjIntObject*
_njlazysweep_newint(NjObject* _vm, int value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  NjIntObject* obj = (NjIntObject*)njord_newint(
      0, value, _njlazysweep_newobj, vm);
  obj->next = vm->startobj;
  vm->startobj = (NjObject*)obj;
  ++vm->objcnt;
  return obj;
}

static NjObject*
njlazysweep_pushint(NjObject* _vm, int value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (vm->objcnt > vm->maxobj)
    _njlazysweep_mark_all(vm);
  return njvm_pushint(_vm, value, 0, _njlazysweep_newint);
}

static NjPairObject*
_njlazysweep_newpair(NjObject* _vm, NjObject* head, NjObject* tail) {
  NjVMObject* vm = (NjVMObject*)_vm;
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      0, head, tail, _njlazysweep_newobj, vm);
  obj->next = vm->startobj;
  vm->startobj = (NjObject*)obj;
  ++vm->objcnt;
  return obj;
}

static NjObject*
njlazysweep_pushpair(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (vm->objcnt >= vm->maxobj)
    _njlazysweep_mark_all(vm);
  return njvm_pushpair(_vm, 0, _njlazysweep_newpair);
}

static NjGCMethods gc_methods = {
  njlazysweep_newvm, /* gc_newvm */
  njlazysweep_freevm, /* gc_freevm */
  njlazysweep_pushint, /* gc_pushint */
  njlazysweep_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  0, /* gc_pop */
  njlazysweep_collect, /* gc_collect */
};

NjTypeObject NjLazy_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "lazysweep_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};
