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
#define LAZY_HEAP_SIZE    (512 << 10)
#define ALIGNMENT         (8)
#define ROUND_UP(n)       (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

static Nj_uchar_t* lazy_heap;
static Nj_uchar_t* allocptr;

static void
_njlazyheap_init(void) {
  lazy_heap = (Nj_uchar_t*)malloc(LAZY_HEAP_SIZE);
  Nj_CHECK(lazy_heap != NULL, "init lazy heap failed");

  allocptr = lazy_heap;
}

static void
_njlazyheap_destroy(void) {
  free(lazy_heap);

  lazy_heap = NULL;
  allocptr = NULL;
}

static NjObject*
njlazysweep_newvm(void) {
  // TODO:
  return NULL;
}

static void
njlazysweep_freevm(NjObject* vm) {
  // TODO:
}

static NjObject*
njlazysweep_pushint(NjObject* vm, int value) {
  // TODO:
  return NULL;
}

static NjObject*
njlazysweep_pushpair(NjObject* vm) {
  // TODO:
  return NULL;
}

static void
njlazysweep_setpair(NjObject* obj, NjObject* head, NjObject* tail) {
  if (head != NULL)
    njord_pairsetter(obj, "head", head);

  if (tail != NULL)
    njord_pairsetter(obj, "tail", tail);
}

static void
njlazysweep_pop(NjObject* vm) {
  // TODO:
}

static void
njlazysweep_collect(NjObject* vm) {
  // TODO:
}

static NjGCMethods gc_methods = {
  njlazysweep_newvm, /* gc_newvm */
  njlazysweep_freevm, /* gc_freevm */
  njlazysweep_pushint, /* gc_pushint */
  njlazysweep_pushpair, /* gc_pushpair */
  njlazysweep_setpair, /* gc_setpair */
  njlazysweep_pop, /* gc_pop */
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
