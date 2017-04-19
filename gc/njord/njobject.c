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
#include "njmem.h"
#include "njobject.h"
#include "njlog.h"
#include "gc_impl.h"

static NjObject gc;

void
njord_initgc(NjGCType type) {
  switch (type) {
  case GC_REFS:
    gc.ob_type = &NjRefs_Type; break;
  case GC_MARK_SWEEP:
    gc.ob_type = &NjMarks_Type; break;
  case GC_MARK_SWEEP2:
    gc.ob_type = &NjMarks2_Type; break;
  case GC_MARK_SWEEP3:
    gc.ob_type = &NjMarks3_Type; break;
  case GC_COPYING:
    gc.ob_type = &NjCopy_Type; break;
  case GC_BITMAP:
    gc.ob_type = &NjBitmap_Type; break;
  case GC_LAZY_SWEEP:
    gc.ob_type = &NjLazy_Type; break;
  case GC_MARK_COMPACTION:
    gc.ob_type = &NjCompaction_Type; break;
  default:
    /* use reference counting gc as defaulted */
    gc.ob_type = &NjRefs_Type; break;
  }
}

NjObject*
njord_new(void) {
  return Nj_GC(&gc)->gc_newvm();
}

void
njord_free(NjObject* vm) {
  Nj_GC(&gc)->gc_freevm(vm);
}

NjObject*
njord_pushint(NjObject* vm, int value) {
  return Nj_GC(&gc)->gc_pushint(vm, value);
}

NjObject*
njord_pushpair(NjObject* vm) {
  return Nj_GC(&gc)->gc_pushpair(vm);
}

void
njord_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (Nj_GC(&gc)->gc_setpair != NULL) {
    Nj_GC(&gc)->gc_setpair(pair, head, tail);
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
  Nj_GC(&gc)->gc_pop(vm);
}

void
njord_collect(NjObject* vm) {
  if (Nj_GC(&gc)->gc_collect != NULL)
    Nj_GC(&gc)->gc_collect(vm);
  else
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
