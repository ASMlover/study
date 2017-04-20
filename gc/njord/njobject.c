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
#include <string.h>
#include "njobject.h"
#include "njlog.h"
#include "njmem.h"
#include "njvm.h"
#include "gc_impl.h"

typedef struct _gcmap {
  NjGCType gc_type;
  NjTypeObject* gc_obj;
} NjGCMap;

static NjGCMap gc_mapped[] = {
  {GC_REFS, &NjRefs_Type},
  {GC_MARK_SWEEP, &NjMarks_Type},
  {GC_MARK_SWEEP2, &NjMarks2_Type},
  {GC_MARK_SWEEP3, &NjMarks3_Type},
  {GC_COPYING, &NjCopy_Type},
  {GC_BITMAP, &NjBitmap_Type},
  {GC_LAZY_SWEEP, &NjLazy_Type},
  {GC_MARK_COMPACTION, &NjCompaction_Type},
  {GC_COPYING2, &NjCopy2_Type},
};

#define Nj_STARTUPGC()  (gc_mapped[startup_gctype].gc_obj->tp_gc)

static NjGCType startup_gctype;
static NjObject* startup_gc;

void
njord_initgc(NjGCType type) {
  Nj_CHECK(type >= 0 && type < GC_COUNTS, "gc type invalid");
  startup_gctype = type;
  startup_gc = NULL;
}

NjObject*
njord_new(void) {
  if (Nj_STARTUPGC()->gc_newvm != NULL)
    startup_gc = Nj_STARTUPGC()->gc_newvm();
  else
    startup_gc = njvm_defgc()->gc_newvm();
  return startup_gc;
}

void
njord_free(NjObject* vm) {
  if (Nj_GC(vm)->gc_freevm != NULL)
    Nj_GC(vm)->gc_freevm(vm);
  else
    njvm_base(vm)->tp_gc->gc_freevm(vm);
}

NjObject*
njord_pushint(NjObject* vm, int value) {
  if (Nj_GC(vm)->gc_pushint != NULL)
    return Nj_GC(vm)->gc_pushint(vm, value);
  else
    return njvm_base(vm)->tp_gc->gc_pushint(vm, value);
}

NjObject*
njord_pushpair(NjObject* vm) {
  if (Nj_GC(vm)->gc_pushpair != NULL)
    return Nj_GC(vm)->gc_pushpair(vm);
  else
    return njvm_base(vm)->tp_gc->gc_pushpair(vm);
}

void
njord_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (Nj_GC(startup_gc)->gc_setpair != NULL) {
    Nj_GC(startup_gc)->gc_setpair(pair, head, tail);
  }
  else if (njvm_base(startup_gc)->tp_gc->gc_setpair != NULL) {
    njvm_base(startup_gc)->tp_gc->gc_setpair(pair, head, tail);
  }
  else if (pair->ob_type->tp_setter != NULL) {
    if (head != NULL)
      pair->ob_type->tp_setter(pair, "head", head);

    if (tail != NULL)
      pair->ob_type->tp_setter(pair, "tail", tail);
  }
  else {
    njlog_fatal("njord_setpair: no matched function\n");
  }
}

void
njord_pop(NjObject* vm) {
  if (Nj_GC(vm)->gc_pop != NULL)
    Nj_GC(vm)->gc_pop(vm);
  else
    njvm_base(vm)->tp_gc->gc_pop(vm);
}

void
njord_collect(NjObject* vm) {
  if (Nj_GC(vm)->gc_collect != NULL)
    Nj_GC(vm)->gc_collect(vm);
  njmem_collect();
}

void
njord_print(NjObject* obj) {
  if (obj->ob_type->tp_print != NULL)
    obj->ob_type->tp_print(obj);
  else
    njlog_repr("NjObject<%p, %s>\n", obj, obj->ob_type->tp_name);
}

NjTypeObject NjType_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "type", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  0, /* tp_gc */
};

static Nj_uchar_t gc_bitmaps[Nj_VMSTACK];

void
njmark_init(void) {
  memset(gc_bitmaps, 0, sizeof(gc_bitmaps));
}

Nj_bool_t
njmark_ismarked(NjObject* obj) {
  return (Nj_bool_t)(gc_bitmaps[njhash_getindex(obj, Nj_VMSTACK)] == MARKED);
}

void
njmark_setmark(NjObject* obj) {
  gc_bitmaps[njhash_getindex(obj, Nj_VMSTACK)] = MARKED;
}

void
njmark_unsetmark(NjObject* obj) {
  gc_bitmaps[njhash_getindex(obj, Nj_VMSTACK)] = UNMARKED;
}
