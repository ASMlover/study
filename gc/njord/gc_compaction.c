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
#include "gc_impl.h"
#include "njlog.h"
#include "njmem.h"

#define MAX_STACK         (1024)
#define INIT_GC_THRESHOLD (64)
#define MAX_GC_THRESHOLD  (1024)
#define COMPACTION_HEAP   (512 << 10)
#define ALIGNMENT         (8)
#define ROUND_UP(n)       (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define Nj_ASGC(ob)       ((GCHead*)(ob) - 1)
#define Nj_FORWARDING(ob)\
  ((NjObject*)(Nj_ASGC(ob)->forwarding + sizeof(GCHead)))

static Nj_uchar_t* heap_address;
static Nj_uchar_t* allocptr;

static void njcompact_collect(NjObject* vm);

static void
_njcompactheap_init(void) {
  heap_address = (Nj_uchar_t*)malloc(COMPACTION_HEAP);
  Nj_CHECK(heap_address != NULL, "allocating heap failed");

  allocptr = heap_address;
}

static void
_njcompactheap_destroy(void) {
  free(heap_address);
}

static void*
_njcompactheap_alloc(Nj_ssize_t n, void* arg) {
  NjObject* vm = (NjObject*)arg;

  void* p = NULL;
  if (allocptr + n > heap_address + COMPACTION_HEAP)
    njcompact_collect(vm);

  if (allocptr + n <= heap_address + COMPACTION_HEAP) {
    p = allocptr;
    allocptr += n;
  }
  Nj_CHECK(p != NULL, "allocating object memory failed");

  return p;
}

typedef enum _marked {
  UNMARKED,
  MARKED,
} NjMarked;

static Nj_uchar_t gc_bitmaps[MAX_GC_THRESHOLD];

static int
_njcompact_ismarked(NjObject* obj) {
  return gc_bitmaps[njhash_getindex(obj, MAX_GC_THRESHOLD)] == MARKED;
}

static void
_njcompact_setmarked(NjObject* obj) {
  gc_bitmaps[njhash_getindex(obj, MAX_GC_THRESHOLD)] = MARKED;
}

/*
 *static void
 *_njcompact_unsetmarked(NjObject* obj) {
 *  gc_bitmaps[njhash_getindex(obj, MAX_GC_THRESHOLD)] = UNMARKED;
 *}
 */

typedef struct _gc {
  Nj_uchar_t* forwarding;
} GCHead;

typedef struct _vm {
  NjObject_HEAD;

  NjObject* stack[MAX_STACK];
  Nj_int_t stackcnt;

  NjObject* startobj;
  NjObject* endobj;
  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

static void
_njcompact_push(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njcompact_pop(NjVMObject* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

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
  if (_njcompact_ismarked(obj))
    return;

  _njcompact_setmarked(obj);
  if (obj->ob_type == &NjPair_Type) {
    NjObject* head = njord_pairgetter(obj, "head");
    if (head != NULL && !_njcompact_ismarked(head))
      _njcompact_mark(head);

    NjObject* tail = njord_pairgetter(obj, "tail");
    if (tail != NULL && !_njcompact_ismarked(tail))
      _njcompact_setmarked(tail);
  }
}

static void
_njcompact_mark_all(NjVMObject* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njcompact_mark(vm->stack[i]);
}

static void
_njcompact_sweep(NjVMObject* vm) {
  Nj_uchar_t* freeptr = heap_address;
  NjObject* scan = vm->startobj;

  /* setting new forwarding address */
  while (scan != NULL) {
    if (_njcompact_ismarked(scan)) {
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
    if (_njcompact_ismarked(scan)) {
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
    if (_njcompact_ismarked(scan)) {
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
  memset(gc_bitmaps, 0, sizeof(gc_bitmaps));
}

static void
njcompact_collect(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  Nj_int_t old_objcnt = vm->objcnt;

  _njcompact_mark_all(vm);
  _njcompact_sweep(vm);

  if (vm->maxobj < MAX_GC_THRESHOLD) {
    vm->maxobj = vm->objcnt << 1;
    if (vm->maxobj > MAX_GC_THRESHOLD)
      vm->maxobj = MAX_GC_THRESHOLD;
  }

  njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
      vm->ob_type->tp_name, old_objcnt - vm->objcnt, vm->objcnt);
}

static NjObject*
njcompact_newvm(void) {
  NjVMObject* vm = (NjVMObject*)njmem_malloc(sizeof(NjVMObject));
  Nj_CHECK(vm != NULL, "create VM failed");

  vm->ob_type = &NjCompaction_Type;
  vm->stackcnt = 0;
  vm->startobj = NULL;
  vm->endobj = NULL;
  vm->objcnt = 0;
  vm->maxobj = INIT_GC_THRESHOLD;
  _njcompactheap_init();

  return (NjObject*)vm;
}

static void
njcompact_freevm(NjObject* vm) {
  njcompact_collect(vm);
  _njcompactheap_destroy();
  njmem_free(vm, sizeof(NjVMObject));
}

static NjObject*
njcompact_pushint(NjObject* _vm, int value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (vm->objcnt >= vm->maxobj)
    njcompact_collect(_vm);

  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, _njcompactheap_alloc, vm);
  _njcompact_insert(vm, (NjObject*)obj);
  ++vm->objcnt;
  _njcompact_push(vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static NjObject*
njcompact_pushpair(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (vm->objcnt >= vm->maxobj)
    njcompact_collect(_vm);

  NjObject* tail = _njcompact_pop(vm);
  NjObject* head = _njcompact_pop(vm);
  NjPairObject* obj = (NjPairObject*)njord_newpair(
      sizeof(GCHead), head, tail, _njcompactheap_alloc, vm);
  _njcompact_insert(vm, (NjObject*)obj);
  ++vm->objcnt;
  _njcompact_push(vm, (NjObject*)obj);

  return (NjObject*)obj;
}

static void
njcompact_setpair(NjObject* obj, NjObject* head, NjObject* tail) {
  if (head != NULL )
    njord_pairsetter(obj, "head", head);

  if (tail != NULL)
    njord_pairsetter(obj, "tail", tail);
}

static void
njcompact_pop(NjObject* vm) {
  _njcompact_pop((NjVMObject*)vm);
}

static NjGCMethods gc_methods = {
  njcompact_newvm, /* gc_newvm */
  njcompact_freevm, /* gc_freevm */
  njcompact_pushint, /* gc_pushint */
  njcompact_pushpair, /* gc_pushpair */
  njcompact_setpair, /* gc_setpair */
  njcompact_pop, /* gc_pop */
  njcompact_collect, /* gc_collect */
};

NjTypeObject NjCompaction_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "markcompaction_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};
