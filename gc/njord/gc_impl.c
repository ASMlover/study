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
#include "njmem.h"

NjTypeObject NjInt_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "int", /* tp_name */
  0, /* tp_print */
  0, /* tp_gc */
};

NjTypeObject NjPair_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "pair", /* tp_name */
  0, /* tp_print */
  0, /* tp_gc */
};

NjObject*
njord_new_object(NjVarType type, Nj_ssize_t gc_size) {
  NjObject* obj = NULL;
  NjTypeObject* typeobj = NULL;
  Nj_ssize_t ob_size = 0;

  if (type == VAR_INT) {
    ob_size = sizeof(NjIntObject);
    typeobj = &NjInt_Type;
  }
  else if (type == VAR_PAIR) {
    ob_size = sizeof(NjPair_Type);
    typeobj = &NjPair_Type;
  }
  Nj_char_t* p = (Nj_char_t*)njmem_malloc(ob_size + gc_size);
  obj = (NjObject*)(p + gc_size);
  obj->ob_type = typeobj;
  ((NjVarObject*)obj)->ob_size = ob_size;

  return obj;
}

void
njord_free_object(NjObject* obj, Nj_ssize_t gc_size) {
  Nj_ssize_t ob_size = ((NjVarObject*)obj)->ob_size;
  Nj_char_t* p = (Nj_char_t*)obj - gc_size;
  njmem_free(p, ob_size + gc_size);
}
