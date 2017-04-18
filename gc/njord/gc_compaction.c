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
#include "gc_impl.h"
#include "njlog.h"
#include "njmem.h"

#define MAX_STACK         (1024)
#define INIT_GC_THRESHOLD (64)
#define MAX_GC_THRESHOLD  (1024)
#define COMPACTION_HEAP   (512 << 10)

typedef struct _block {
  struct _block* nextblock;
  Nj_ssize_t blocksz;
} Block;

static Nj_uchar_t* heap_address;
static Nj_uchar_t* allocptr;
static Block* freeblock;

static void
_njcompactheap_init(void) {
  heap_address = (Nj_uchar_t*)malloc(COMPACTION_HEAP);
  Nj_CHECK(heap_address != NULL, "allocating heap failed");

  allocptr = heap_address;
  freeblock = NULL;
}

static void
_njcompactheap_destroy(void) {
  free(heap_address);

  heap_address = NULL;
  allocptr = NULL;
  freeblock = NULL;
}

static void*
_njcompactheap_alloc(Nj_ssize_t n) {
  void* p = NULL;
  if (freeblock != NULL) {
    Block** block = &freeblock;
    while (*block != NULL) {
      if ((*block)->blocksz >= n) {
        p = *block;
        *block = (*block)->nextblock;
      }
      else {
        block = &(*block)->nextblock;
      }
    }
  }

  if (p == NULL && allocptr + n <= heap_address + COMPACTION_HEAP) {
    p = allocptr;
    allocptr += n;
  }

  return p;
}

typedef enum _marked {
  UNMARKED,
  MARKED,
} NjMarked;

static Nj_uchar_t gc_bitmaps[MAX_GC_THRESHOLD];

static inline int
_hash_index(NjObject* obj) {
  Nj_size_t h = 1315423911;
  h ^= ((h << 5) + (Nj_size_t)obj + (h >> 2));
  return h % MAX_GC_THRESHOLD;
}

static int
_njcompact_ismarked(NjObject* obj) {
  return gc_bitmaps[_hash_index(obj)] == MARKED;
}

static void
_njcompact_setmarked(NjObject* obj) {
  gc_bitmaps[_hash_index(obj)] = MARKED;
}

static void
_njcompact_unsetmarked(NjObject* obj) {
  gc_bitmaps[_hash_index(obj)] = UNMARKED;
}

typedef struct _vm {
  NjObject_HEAD;

  NjObject* stack[MAX_STACK];
  Nj_int_t stackcnt;

  NjObject* startobj;
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
  // TODO:
}

static void
njcompact_collect(NjObject* vm) {
  // TODO:
}

static NjObject*
njcompact_newvm(void) {
  NjVMObject* vm = (NjVMObject*)njmem_malloc(sizeof(NjVMObject));
  Nj_CHECK(vm != NULL, "create VM failed");

  vm->ob_type = &NjCompaction_Type;
  vm->stackcnt = 0;
  vm->startobj = NULL;
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

  NjIntObject* obj = (NjIntObject*)njord_newint(0, value, NULL, NULL);
  obj->next = vm->startobj;
  vm->startobj = (NjObject*)obj;
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
  NjPairObject* obj = (NjPairObject*)njord_newpair(0, head, tail, NULL, NULL);
  obj->next = vm->startobj;
  vm->startobj = (NjObject*)obj;
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
