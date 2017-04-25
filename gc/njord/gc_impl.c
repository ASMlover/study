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
#include <stdio.h>
#include <string.h>
#include "gc_impl.h"
#include "njlog.h"
#include "njmem.h"

static void
njint_print(NjObject* obj) {
  njlog_repr("NjIntObject<`%s`, %p, %d>\n",
      obj->ob_type->tp_name, obj, ((NjIntObject*)obj)->value);
}

static void
njint_debug(NjObject* obj) {
  njlog_debug("NjIntObject<`%s`, %p, %d>\n",
      obj->ob_type->tp_name, obj, ((NjIntObject*)obj)->value);
}

NjTypeObject NjInt_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "int", /* tp_name */
  njint_print, /* tp_print */
  njint_debug, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  0, /* tp_gc */
};

static void
njpair_print(NjObject* obj) {
  NjObject* head = ((NjPairObject*)obj)->head;
  NjObject* tail = ((NjPairObject*)obj)->tail;
  njlog_repr("NjPairObject<`%s`, %p, <<`%s`, %p>, <`%s`, %p>>>\n",
      obj->ob_type->tp_name, obj,
      head != NULL ? head->ob_type->tp_name : "null", head,
      tail != NULL ? tail->ob_type->tp_name : "null", tail);
}

static void
njpair_debug(NjObject* obj) {
  NjObject* head = ((NjPairObject*)obj)->head;
  NjObject* tail = ((NjPairObject*)obj)->tail;
  njlog_debug("NjPairObject<`%s`, %p, <<`%s`, %p>, <`%s`, %p>>>\n",
      obj->ob_type->tp_name, obj,
      head != NULL ? head->ob_type->tp_name : "null", head,
      tail != NULL ? tail->ob_type->tp_name : "null", tail);
}

NjTypeObject NjPair_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "pair", /* tp_name */
  njpair_print, /* tp_print */
  njpair_debug, /* tp_debug */
  njord_pairsetter, /* tp_setter */
  njord_pairgetter, /* tp_getter */
  0, /* tp_gc */
};

NjObject*
njord_newint(Nj_ssize_t gc_size,
    Nj_int_t value, allocfunc user_alloc, void* arg) {
  Nj_ssize_t ob_size = sizeof(NjIntObject);
  Nj_char_t* p;
  if (user_alloc != NULL)
    p = (Nj_char_t*)user_alloc(ob_size + gc_size, arg);
  else
    p = (Nj_char_t*)njmem_malloc(ob_size + gc_size);
  NjIntObject* obj = (NjIntObject*)(p + gc_size);
  obj->ob_type = &NjInt_Type;
  obj->ob_size = ob_size;
  obj->value = value;

  return Nj_ASOBJ(obj);
}

NjObject*
njord_newpair(Nj_ssize_t gc_size, allocfunc user_alloc, void* arg) {
  Nj_ssize_t ob_size = sizeof(NjPairObject);
  Nj_char_t* p;
  if (user_alloc != NULL)
    p = (Nj_char_t*)user_alloc(ob_size + gc_size, arg);
  else
    p = (Nj_char_t*)njmem_malloc(ob_size + gc_size);
  NjPairObject* obj = (NjPairObject*)(p + gc_size);
  obj->ob_type = &NjPair_Type;
  obj->ob_size = ob_size;

  return Nj_ASOBJ(obj);
}

void
njord_pairsetter(NjObject* obj, const char* key, NjObject* value) {
  if (strcmp(key, "head") == 0)
    ((NjPairObject*)obj)->head = value;
  else if (strcmp(key, "tail") == 0)
    ((NjPairObject*)obj)->tail = value;
}

NjObject*
njord_pairgetter(NjObject* obj, const char* key) {
  if (strcmp(key, "head") == 0)
    return ((NjPairObject*)obj)->head;
  else if (strcmp(key, "tail") == 0)
    return ((NjPairObject*)obj)->tail;
  return NULL;
}

void
njord_freeobj(NjObject* obj, Nj_ssize_t gc_size) {
  Nj_ssize_t ob_size = ((NjVarObject*)obj)->ob_size;
  Nj_char_t* p = (Nj_char_t*)obj - gc_size;
  njmem_free(p, ob_size + gc_size);
}
