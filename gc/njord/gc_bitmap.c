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
#include "njvm.h"

#define Nj_GC_INITTHRESHOLD (64)
#define Nj_GC_MAXTHRESHOLD  (1024)
#define Nj_GC_BITMAPS       (Nj_GC_MAXTHRESHOLD >> 3)
#define Nj_BIT_BIG(i)       ((i) / 8)
#define Nj_BIT_SMALL(i)     ((i) % 8)
#define Nj_BIT_GET(i)\
  ((gc_bitmaps[Nj_BIT_BIG(i)] >> Nj_BIT_SMALL(i)) & 1)
#define Nj_BIT_SET(i)\
  (gc_bitmaps[Nj_BIT_BIG(i)] |= (1 << Nj_BIT_SMALL(i)))
#define Nj_BIT_CLR(i)\
  (gc_bitmaps[Nj_BIT_BIG(i)] &= ~(1 << Nj_BIT_SMALL(i)))

typedef struct _vm {
  NjObject_VM_HEAD;

  NjObject* startobj;
  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

static Nj_uchar_t gc_bitmaps[Nj_GC_BITMAPS];

static void
_njbitmap_mark(NjObject* obj) {
  int i = njhash_getindex(obj, Nj_GC_MAXTHRESHOLD);
  if (Nj_BIT_GET(i))
    return;

  Nj_BIT_SET(i);
  if (obj->ob_type == &NjPair_Type) {
    NjObject* head = njord_pairgetter(obj, "head");
    _njbitmap_mark(head);
    NjObject* tail = njord_pairgetter(obj, "tail");
    _njbitmap_mark(tail);
  }
}

static void
_njbitmap_mark_all(NjVMObject* vm) {
  for (int i = 0; i < vm->stackcnt; ++i)
    _njbitmap_mark(vm->stack[i]);
}

static void
_njbitmap_sweep(NjVMObject* vm) {
  NjObject** startobj = &vm->startobj;
  while (*startobj != NULL) {
    int i = njhash_getindex(*startobj, Nj_GC_MAXTHRESHOLD);
    if (!Nj_BIT_GET(i)) {
      NjObject* unmarked = *startobj;
      *startobj = ((NjVarObject*)unmarked)->next;
      njlog_debug("NjObject<%p, '%s'> collected\n",
          unmarked, unmarked->ob_type->tp_name);
      njord_freeobj(unmarked, 0);
      --vm->objcnt;
    }
    else {
      Nj_BIT_CLR(i);
      startobj = &((NjVarObject*)*startobj)->next;
    }
  }
}

static NjIntObject*
_njbitmap_newint(NjObject* vm, Nj_int_t value) {
  NjIntObject* obj = (NjIntObject*)njord_newint(0, value, NULL, NULL);
  obj->next = Nj_VM(vm)->startobj;
  Nj_VM(vm)->startobj = (NjObject*)obj;
  ++Nj_VM(vm)->objcnt;
  return obj;
}

static NjObject*
njbitmap_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(
      vm, value, Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njbitmap_newint);
}

static NjPairObject*
_njbitmap_newpair(NjObject* vm, NjObject* head, NjObject* tail) {
  NjPairObject* obj = (NjPairObject*)njord_newpair(0, head, tail, NULL, NULL);
  obj->next = Nj_VM(vm)->startobj;
  Nj_VM(vm)->startobj = (NjObject*)obj;
  ++Nj_VM(vm)->objcnt;
  return obj;
}

static NjObject*
njbitmap_pushpair(NjObject* vm) {
  return njvm_pushpair(
      vm, Nj_VM(vm)->objcnt >= Nj_VM(vm)->maxobj, _njbitmap_newpair);
}

static void
njbitmap_collect(NjObject* vm) {
  Nj_int_t old_objcnt = Nj_VM(vm)->objcnt;

  _njbitmap_mark_all(Nj_VM(vm));
  _njbitmap_sweep(Nj_VM(vm));

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
  0, /* gc_dealloc */
  njbitmap_pushint, /* gc_pushint */
  njbitmap_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  0, /* gc_pop */
  njbitmap_collect, /* gc_collect */
};

static NjTypeObject NjBitmap_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "markbitmap_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njbitmap_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjBitmap_Type;
  Nj_VM(vm)->startobj = NULL;
  Nj_VM(vm)->objcnt = 0;
  Nj_VM(vm)->maxobj = Nj_GC_INITTHRESHOLD;
}

NjObject*
njbitmap_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njbitmap_vm_init);
}
