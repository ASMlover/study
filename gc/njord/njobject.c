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
#include "njvm.h"
#include "gc_impl.h"

typedef NjObject* (*newvmfunc)(void);
typedef struct _gcmap {
  const char* gc_name;
  const char* gc_doc;
  newvmfunc gc_newvm;
} NjGCMap;

static NjGCMap gc_mapped[] = {
  {"refs",
    "\t[refs] - reference-counting garbage collector",
    njrefs_create},
  {"marks",
    "\t[marks] - mark-sweep garbage collector version-1",
    njmarks_create},
  {"marks2",
    "\t[marks2] - mark-sweep garbage collector version-2",
    njmarks2_create},
  {"marks3",
    "\t[marks3] - mark-sweep garbage collector version-3",
    njmarks3_create},
  {"copy",
    "\t[copy] - semispaces-copying garbage collector version-1",
    njsemispacecopy_create},
  {"bits",
    "\t[bits] - mark-sweep with bitmap garbage collector",
    njbitmap_create},
  {"lazy",
    "\t[lazy] - lazy-sweep garbage collector",
    njlazysweep_create},
  {"compact",
    "\t[compact] - mark-compaction garbage collector",
    njcompact_create},
  {"copy2",
    "\t[copy2] - semispaces-copying garbage collector version-2",
    njsemispacecopy2_create},
  {"refs2",
    "\t[refs2] - reference-counting garbage collector version-2",
    njrefs2_create},
};

#define Nj_STARTUPGC()    (gc_mapped[startup_gcindex])
#define Nj_GCCOUNT(array) ((int)(sizeof(array) / sizeof(array[0])))

static int startup_gcindex;
static NjObject* startup_gc;

void
njord_initgc(const char* name) {
  startup_gcindex = -1;
  startup_gc = NULL;

  for (int i = 0; i < Nj_GCCOUNT(gc_mapped); ++i) {
    if (strcmp(gc_mapped[i].gc_name, name) == 0) {
      startup_gcindex = i;
      break;
    }
  }
  if (startup_gcindex == -1)
    njord_usagegc();
  Nj_CHECK(startup_gcindex != -1, "startup gc failed");
}

void
njord_usagegc(void) {
  njlog_repr(
      "USAGE: njord gc [name] [profile]\n"
      " name:\n");
  for (int i = 0; i < Nj_GCCOUNT(gc_mapped); ++i)
    njlog_repr("%s\n", gc_mapped[i].gc_doc);
  njlog_repr(
      " profile:\n"
      "\t[0] - withnot performace testing\n"
      "\t[1] - with performace testing\n");
}

NjObject*
njord_new(void) {
  if (Nj_STARTUPGC().gc_newvm != NULL)
    startup_gc = Nj_STARTUPGC().gc_newvm();
  else
    startup_gc = njvm_defvm();
  return startup_gc;
}

void
njord_free(NjObject* vm) {
  if (Nj_GC(vm)->gc_dealloc != NULL)
    Nj_GC(vm)->gc_dealloc(vm);
  else
    njvm_base(vm)->tp_gc->gc_dealloc(vm);
}

NjObject*
njord_pushint(NjObject* vm, Nj_int_t value) {
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
  else
    njvm_base(vm)->tp_gc->gc_collect(vm);
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
