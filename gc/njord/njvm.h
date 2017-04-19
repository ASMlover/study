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
#ifndef Nj_NJVM_H
#define Nj_NJVM_H

#include "gc_impl.h"

#define Nj_VMSTACK  (1024)
#define NjObject_VM_HEAD\
  NjObject_HEAD;\
  NjTypeObject* vm_base;\
  Nj_ssize_t vm_size;\
  NjObject* stack[Nj_VMSTACK];\
  Nj_int_t stackcnt

typedef void (*initvmfunc)(NjObject*);
typedef void (*destroyvmfunc)(NjObject*);
typedef NjIntObject* (newintfunc)(NjObject*, int);
typedef NjPairObject* (newpairfunc)(NjObject*, NjObject*, NjObject*);

NjAPI_FUNC(NjGCMethods*) njvm_defgc(void);
NjAPI_FUNC(NjTypeObject*) njvm_base(NjObject* vm);
NjAPI_FUNC(NjObject*) njvm_newvm(Nj_ssize_t vmsz, initvmfunc init);
NjAPI_FUNC(void) njvm_freevm(NjObject* vm, destroyvmfunc destroy);
NjAPI_FUNC(NjObject*) njvm_pushint(NjObject* vm,
    int value, int need_collect, newintfunc newint);
NjAPI_FUNC(NjObject*) njvm_pushpair(
    NjObject* vm, int need_collect, newpairfunc newpair);

#endif /* Nj_NJVM_H */
