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
#include "njlog.h"
#include "njmem.h"
#include "njvm.h"

#define Nj_ASGC(ob) ((GCHead*)(ob) - 1)

typedef struct _gc {
  Nj_uchar_t marked;
} GCHead;

typedef struct _vm {
  NjObject_VM_HEAD;

  NjObject* startobj;
  Nj_int_t objcnt;
  Nj_int_t maxobj;
} NjVMObject;

typedef struct _node {
  struct _node* next;
  NjObject* obj;
} WorkNode;

static WorkNode* worklist = NULL;

static void
_njmarks_worklist_init(void) {
  worklist = NULL;
}

static void
_njmarks_worklist_push(NjObject* obj) {
  WorkNode* node = (WorkNode*)njmem_malloc(sizeof(WorkNode));
  node->obj = obj;
  node->next = worklist;
  worklist = node;
}

static NjObject*
_njmarks_worklist_pop(void) {
  WorkNode* node = worklist;
  NjObject* obj = node->obj;
  worklist = worklist->next;
  njmem_free(node, sizeof(*node));

  return obj;
}

static void
_njmarks_mark(void) {
  while (worklist != NULL) {
    NjObject* ref = _njmarks_worklist_pop();
    if (ref->ob_type == &NjPair_Type) {
      NjObject* head = njord_pairgetter(ref, "head");
      if (head != NULL && Nj_ASGC(head)->marked == UNMARKED) {
        Nj_ASGC(head)->marked = MARKED;
        _njmarks_worklist_push(head);
      }

      NjObject* tail = njord_pairgetter(ref, "tail");
      if (tail != NULL && Nj_ASGC(tail)->marked == UNMARKED) {
        Nj_ASGC(tail)->marked = MARKED;
        _njmarks_worklist_push(tail);
      }
    }
  }
}

static void
_njmarks_mark_all(NjVMObject* vm) {
  _njmarks_worklist_init();
  for (int i = 0; i < vm->stackcnt; ++i) {
    NjObject* ref = vm->stack[i];
    if (ref != NULL && Nj_ASGC(ref)->marked == UNMARKED) {
      Nj_ASGC(ref)->marked = MARKED;
      _njmarks_worklist_push(ref);
      _njmarks_mark();
    }
  }
}

static void
_njmarks_sweep(NjVMObject* vm) {
  NjObject** startobj = &vm->startobj;
  while (*startobj != NULL) {
    if (Nj_ASGC(*startobj)->marked == UNMARKED) {
      NjObject* unmarked = *startobj;
      *startobj = ((NjVarObject*)unmarked)->next;
      njlog_debug("NjObject<%p, '%s'> collected.\n",
          unmarked, unmarked->ob_type->tp_name);
      njord_freeobj(unmarked, sizeof(GCHead));
      --vm->objcnt;
    }
    else {
      Nj_ASGC(*startobj)->marked = UNMARKED;
      startobj = &((NjVarObject*)*startobj)->next;
    }
  }
}

static NjIntObject*
_njmarks_newint(NjObject* _vm, Nj_int_t value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  NjIntObject* obj = (NjIntObject*)njord_newint(
      sizeof(GCHead), value, NULL, NULL);
  Nj_ASGC(obj)->marked = UNMARKED;
  obj->next = vm->startobj;
  vm->startobj = (NjObject*)obj;
  ++vm->objcnt;
  return obj;
}

static NjObject*
njmarks_pushint(NjObject* _vm, Nj_int_t value) {
  NjVMObject* vm = (NjVMObject*)_vm;
  return njvm_pushint(_vm, value, vm->objcnt >= vm->maxobj, _njmarks_newint);
}

static NjPairObject*
_njmarks_newpair(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  NjPairObject* obj = (NjPairObject*)njord_newpair(sizeof(GCHead), NULL, NULL);
  Nj_ASGC(obj)->marked = UNMARKED;
  obj->next = vm->startobj;
  vm->startobj = (NjObject*)obj;
  ++vm->objcnt;
  return obj;
}

static NjObject*
njmarks_pushpair(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  return njvm_pushpair(_vm, vm->objcnt >= vm->maxobj, _njmarks_newpair, NULL);
}

static void
njmarks_collect(NjObject* _vm) {
  NjVMObject* vm = (NjVMObject*)_vm;
  Nj_int_t old_objcnt = vm->objcnt;

  _njmarks_mark_all(vm);
  _njmarks_sweep(vm);

  if (vm->maxobj < Nj_GC_MAXTHRESHOLD) {
    vm->maxobj = vm->objcnt << 1;
    if (vm->maxobj > Nj_GC_MAXTHRESHOLD)
      vm->maxobj = Nj_GC_MAXTHRESHOLD;
  }

  njlog_info("<%s> collected [%d] objects, [%d] remaining.\n",
      vm->ob_type->tp_name, old_objcnt - vm->objcnt, vm->objcnt);
}

static NjGCMethods gc_methods = {
  0, /* gc_dealloc */
  njmarks_pushint, /* gc_pushint */
  njmarks_pushpair, /* gc_pushpair */
  0, /* gc_setpair */
  0, /* gc_pop */
  njmarks_collect, /* gc_collect */
};

static NjTypeObject NjMarks3_Type = {
  NjObject_HEAD_INIT(&NjType_Type),
  "marksweep3_gc", /* tp_name */
  0, /* tp_print */
  0, /* tp_debug */
  0, /* tp_setter */
  0, /* tp_getter */
  (NjGCMethods*)&gc_methods, /* tp_gc */
};

static void
_njmarks_vm_init(NjObject* vm) {
  Nj_VM(vm)->ob_type = &NjMarks3_Type;
  Nj_VM(vm)->startobj = NULL;
  Nj_VM(vm)->objcnt = 0;
  Nj_VM(vm)->maxobj = Nj_GC_INITTHRESHOLD;
}

NjObject*
njmarks3_create(void) {
  return njvm_newvm(sizeof(NjVMObject), _njmarks_vm_init);
}
