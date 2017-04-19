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

typedef struct _vm {
  NjObject_VM_HEAD;
} NjVMObject;

static void
_njdefvm_push(NjVMObject* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < Nj_VMSTACK, "vm stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njdefvm_pop(NjVMObject* vm) {
  Nj_CHECK(vm->stackcnt > 0, "vm stack underflow");
  return vm->stack[--vm->stackcnt];
}

static NjObject*
njdefvm_newvm(void) {
  return njvm_newvm(0, NULL);
}

static void
njdefvm_freevm(NjObject* vm) {
  njvm_freevm(vm, NULL);
}

static NjObject*
njdefvm_pushint(NjObject* vm, int value) {
  return njvm_pushint(vm, value, 0, NULL);
}

static NjObject*
njdefvm_pushpair(NjObject* vm) {
  return njvm_pushpair(vm, 0, NULL);
}

static void
njdefvm_pop(NjObject* vm) {
  _njdefvm_pop((NjVMObject*)vm);
}

static NjGCMethods gc_methods = {
  njdefvm_newvm, /* gc_newvm */
  njdefvm_freevm, /* gc_freevm */
  njdefvm_pushint, /* gc_pushint */
  njdefvm_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  njdefvm_pop, /* gc_pop */
  0, /* gc_collect */
};

static NjTypeObject NjVM_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "defvm", /* tp_name */
  0, /* tp_print */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

NjGCMethods*
njvm_defgc(void) {
  return NjVM_Type.tp_gc;
}

NjTypeObject*
njvm_base(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  return vm->vm_base;
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
    vm->ob_type = &NjVM_Type;

  return (NjObject*)vm;
}

void
njvm_freevm(NjObject* _vm, destroyvmfunc destroy) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (vm->ob_type->tp_gc->gc_collect)
    vm->ob_type->tp_gc->gc_collect(_vm);

  if (destroy != NULL)
    destroy(_vm);

  njmem_free(vm, vm->vm_size);
}

NjObject*
njvm_pushint(NjObject* _vm, int value, int need_collect, newintfunc newint) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (need_collect)
    vm->ob_type->tp_gc->gc_collect(_vm);

  NjIntObject* obj;
  if (newint != NULL)
    obj = newint(_vm, value);
  else
    obj = (NjIntObject*)njord_newint(0, value, NULL, NULL);
  _njdefvm_push(vm, (NjObject*)obj);

  return (NjObject*)obj;
}

NjObject*
njvm_pushpair(NjObject* _vm, int need_collect, newpairfunc newpair) {
  NjVMObject* vm = (NjVMObject*)_vm;
  if (need_collect)
    vm->ob_type->tp_gc->gc_collect(_vm);

  NjPairObject* obj;
  NjObject* tail = _njdefvm_pop(vm);
  NjObject* head = _njdefvm_pop(vm);
  if (newpair != NULL)
    obj = newpair(_vm, head, tail);
  else
    obj = (NjPairObject*)njord_newpair(0, head, tail, NULL, NULL);
  _njdefvm_push(vm, (NjObject*)obj);

  return (NjObject*)obj;
}
