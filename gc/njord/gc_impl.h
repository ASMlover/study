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
#ifndef Nj_GCIMPL_H
#define Nj_GCIMPL_H

#include "njobject.h"

typedef struct _intobject {
  NjObject_VAR_HEAD;
  Nj_int_t value;
} NjIntObject;

typedef struct _pairobject {
  NjObject_VAR_HEAD;
  NjObject* head;
  NjObject* tail;
} NjPairObject;

NjAPI_DATA(NjTypeObject) NjInt_Type;
NjAPI_DATA(NjTypeObject) NjPair_Type;

NjAPI_FUNC(NjObject*) njord_new_object(NjVarType type, Nj_ssize_t gc_size);
NjAPI_FUNC(void) njord_free_object(NjObject* obj, Nj_ssize_t gc_size);

NjAPI_DATA(NjTypeObject) NjRefs_Type; /* easy implementation of reference
                                         counting garbage collector*/
NjAPI_DATA(NjGCObject) NjGC_MarkS; /* easy implementation of mark and
                                      sweep garbage collector */


#endif /* Nj_GCIMPL_H */
