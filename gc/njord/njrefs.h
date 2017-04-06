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
#ifndef Nj_NJREFS_H
#define Nj_NJREFS_H

#include "njconfig.h"

typedef struct _object {
  Nj_ssize_t refcnt;

  NjType type;
  union {
    int value;
    struct {
      struct _object* head;
      struct _object* tail;
    };
  };
} NjObject;

typedef struct _vm NjVM;

NjAPI_FUNC(NjVM*) njref_new(void);
NjAPI_FUNC(void) njref_free(NjVM* vm);
NjAPI_FUNC(NjObject*) njref_pushint(NjVM* vm, int value);
NjAPI_FUNC(NjObject*) njref_pushpair(NjVM* vm);
NjAPI_FUNC(void) njref_setpair(NjObject* pair, NjObject* head, NjObject* tail);
NjAPI_FUNC(void) njref_pop(NjVM* vm);
NjAPI_FUNC(void) njref_collect(NjVM* vm);

#endif /* Nj_NJREFS_H */
