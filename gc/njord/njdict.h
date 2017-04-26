// Copyright (c) 2017 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef Nj_NJDICT_H
#define Nj_NJDICT_H

#include "njobject.h"

typedef struct _dict NjDict;

/* dict length should less than 1024, later we'll add resize functional */
NjAPI_FUNC(NjDict*) njdict_create(void);
NjAPI_FUNC(void) njdict_dealloc(NjDict* dict);
NjAPI_FUNC(void) njdict_clear(NjDict* dict);
NjAPI_FUNC(Nj_ssize_t) njdict_size(NjDict* dict);
NjAPI_FUNC(void) njdict_add(NjDict* dict, NjObject* obj);
NjAPI_FUNC(void) njdict_remove(NjDict* dict, NjObject* obj);
NjAPI_FUNC(NjObject*) njdict_pop(NjDict* dict);
NjAPI_FUNC(void) njdict_traverse(NjDict* dict, visitfunc visit, void* arg);

#endif /* Nj_NJDICT_H */
