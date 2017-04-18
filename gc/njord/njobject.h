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
#ifndef Nj_NJOBJECT_H
#define Nj_NJOBJECT_H

#include "njconfig.h"

#define NjObject_HEAD\
  struct _typeobject* ob_type

#define NjObject_HEAD_INIT(type)\
  type

#define NjObject_VAR_HEAD\
  NjObject_HEAD;\
  Nj_ssize_t ob_size;\
  struct _object* next

typedef struct _object {
  NjObject_HEAD;
} NjObject;

typedef struct _varobject {
  NjObject_VAR_HEAD;
} NjVarObject;

typedef void (*printfunc)(NjObject*);
typedef void (*setterfunc)(NjObject*, const char*, NjObject*);
typedef NjObject* (*getterfunc)(NjObject*, const char*);
typedef NjObject* (*newvmfunc)(void);
typedef void (*freevmfunc)(NjObject*);
typedef NjObject* (*pushintfunc)(NjObject*, int);
typedef NjObject* (*pushpairfunc)(NjObject*);
typedef void (*setpairfunc)(NjObject*, NjObject*, NjObject*);
typedef void (*popfunc)(NjObject*);
typedef void (*collectfunc)(NjObject*);

typedef struct {
  newvmfunc gc_newvm;
  freevmfunc gc_freevm;
  pushintfunc gc_pushint;
  pushpairfunc gc_pushpair;
  setpairfunc gc_setpair;
  popfunc gc_pop;
  collectfunc gc_collect;
} NjGCMethods;

typedef struct _typeobject {
  NjObject_HEAD;
  const char* tp_name;
  printfunc tp_print;
  setterfunc tp_setter;
  getterfunc tp_getter;
  NjGCMethods* tp_gc;
} NjTypeObject;

typedef enum _gctype {
  GC_REFS, /* reference counting garbage collection */
  GC_MARK_SWEEP, /* mark and sweep garbage collection */
  GC_MARK_SWEEP2, /* mark and sweep garbage collection with no-recursive */
  GC_MARK_SWEEP3, /* mark and sweep garbage collection with tri-coloured */
  GC_COPYING, /* copying node garbage collection */
  GC_BITMAP, /* mark and sweep garbage collection with bitmap */
  GC_LAZY_SWEEP, /* lazy sweep garbage collection */
  GC_MARK_COMPACTION, /* mark and compaction garbage collection  */
} NjGCType;

typedef struct _gcobject {
  NjGCType type;
  NjGCMethods* methods;
} NjGCObject;

#define Nj_GC(gc) (((NjObject*)(gc))->ob_type->tp_gc)

NjAPI_DATA(NjTypeObject) NjType_Type;

NjAPI_FUNC(void) njord_initgc(NjGCType type);
NjAPI_FUNC(NjObject*) njord_new(void);
NjAPI_FUNC(void) njord_free(NjObject* vm);
NjAPI_FUNC(NjObject*) njord_pushint(NjObject* vm, int value);
NjAPI_FUNC(NjObject*) njord_pushpair(NjObject* vm);
NjAPI_FUNC(void) njord_setpair(NjObject* pair, NjObject* head, NjObject* tail);
NjAPI_FUNC(void) njord_pop(NjObject* vm);
NjAPI_FUNC(void) njord_collect(NjObject* vm);
NjAPI_FUNC(void) njord_print(NjObject* obj);

#endif /* Nj_NJOBJECT_H */
