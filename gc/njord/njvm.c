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
#include "njvm.h"

typedef struct _vmobject {
  NjObject_VM_HEAD;
} NjVMObject;

static inline void
_njdefvm_push(NjObject* vm, NjObject* obj) {
  Nj_CHECK(Nj_VM(vm)->stackcnt < Nj_VMSTACK, "vm stack overflow");
  Nj_VM(vm)->stack[Nj_VM(vm)->stackcnt++] = obj;
}

static inline NjObject*
_njdefvm_pop(NjObject* vm) {
  Nj_CHECK(Nj_VM(vm)->stackcnt > 0, "vm stack underflow");
  return Nj_VM(vm)->stack[--Nj_VM(vm)->stackcnt];
}

static void
njdefvm_dealloc(NjObject* vm) {
  njvm_freevm(vm, NULL);
}

static NjObject*
njdefvm_pushint(NjObject* vm, Nj_int_t value) {
  return njvm_pushint(vm, value, FALSE, NULL);
}

static NjObject*
njdefvm_pushpair(NjObject* vm) {
  return njvm_pushpair(vm, FALSE, NULL, NULL);
}

static void
njdefvm_pop(NjObject* vm) {
  _njdefvm_pop(vm);
}

static void
njdefvm_collect(NjObject* vm) {
  Nj_UNUSED(vm);
  njmem_collect();
}

static NjGCMethods gc_methods = {
  njdefvm_dealloc, /* gc_dealloc */
  njdefvm_pushint, /* gc_pushint */
  njdefvm_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  njdefvm_pop, /* gc_pop */
  njdefvm_collect, /* gc_collect */
};

static NjTypeObject NjVM_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "defvm_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

NjObject*
njvm_defvm(void) {
  return njvm_newvm(0, NULL);
}

NjTypeObject*
njvm_base(NjObject* vm) {
  return Nj_VM(vm)->vm_base;
}

NjObject*
njvm_newvm(Nj_ssize_t vmsz, initvmfunc init) {
  if (vmsz == 0)
    vmsz = sizeof(NjVMObject);

  NjVMObject* vm = (NjVMObject*)njmem_malloc(vmsz);
  Nj_CHECK(vm != NULL, "create vm failed");

  vm->vm_base = &NjVM_Type;
  vm->vm_size = vmsz;
  vm->stackcnt = 0;
  if (init != NULL)
    init((NjObject*)vm);
  else
    vm->ob_type = &NjType_Type;

  return Nj_ASOBJ(vm);
}

void
njvm_freevm(NjObject* vm, destroyvmfunc destroy) {
  if (Nj_GC(vm)->gc_collect)
    Nj_GC(vm)->gc_collect(vm);

  if (destroy != NULL)
    destroy(vm);

  njmem_free(vm, Nj_VM(vm)->vm_size);
}

NjObject*
njvm_pushint(NjObject* vm,
    Nj_int_t value, Nj_bool_t need_collect, newintfunc newint) {
  if (need_collect)
    Nj_GC(vm)->gc_collect(vm);

  NjIntObject* obj;
  if (newint != NULL)
    obj = newint(vm, value);
  else
    obj = (NjIntObject*)njord_newint(0, value, NULL, NULL);
  _njdefvm_push(vm, Nj_ASOBJ(obj));

  return Nj_ASOBJ(obj);
}

NjObject*
njvm_pushpair(NjObject* vm, Nj_bool_t need_collect,
    newpairfunc newpair, pairinitfunc initpair) {
  if (need_collect)
    Nj_GC(vm)->gc_collect(vm);

  NjPairObject* obj;
  if (newpair != NULL)
    obj = newpair(vm);
  else
    obj = (NjPairObject*)njord_newpair(0, NULL, NULL);
  NjObject* tail = _njdefvm_pop(vm);
  NjObject* head = _njdefvm_pop(vm);
  if (initpair != NULL)
    initpair(vm, obj, head, tail);
  obj->head = head;
  obj->tail = tail;
  _njdefvm_push(vm, Nj_ASOBJ(obj));

  return Nj_ASOBJ(obj);
}

NjObject*
njvm_pop(NjObject* vm) {
  return _njdefvm_pop(vm);
}
