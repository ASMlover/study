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
#include <stdlib.h>
#include "njmem.h"
#include "gc_impl.h"

#define MAX_STACK (1024)

#define Nj_REFCNT(ob) (((NjRefsObject*)(ob))->refcnt)
#define Nj_NEWREF(ob) (Nj_REFCNT(ob) = 1)
#define Nj_INCREF(ob) (((NjRefsObject*)(ob))->refcnt++)
#define Nj_DECREF(ob) do {\
  if (--((NjRefsObject*)(ob))->refcnt == 0) {\
    fprintf(stdout, "NjObject<0x%p, %s, %d> collected\n",\
        ((NjRefsObject*)(ob)),\
        ((NjRefsObject*)(ob))->ob_name,\
        ((NjRefsObject*)(ob))->type);\
    _njord_free_object(ob);\
  }\
} while (0)
#define Nj_XINCREF(ob) do { if ((ob) != NULL) Nj_INCREF(ob); } while (0)
#define Nj_XDECREF(ob) do { if ((ob) != NULL) Nj_DECREF(ob); } while (0)

typedef struct _varobject {
  NjObject_HEAD

  Nj_ssize_t refcnt;
  NjVarType type;
  union {
    int value;
    struct {
      struct _varobject* head;
      struct _varobject* tail;
    };
  };
} NjRefsObject;

typedef struct _vm {
  NjRefsObject* stack[MAX_STACK];
  int stackcnt;
} NjRefsVM;

static void
_njord_push(NjRefsVM* vm, NjRefsObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjRefsObject*
_njord_pop(NjRefsVM* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static NjRefsObject*
_njord_new_object(NjVarType type) {
  NjRefsObject* obj = (NjRefsObject*)njmem_malloc(sizeof(NjRefsObject));
  obj->ob_name = "refs_object";
  Nj_NEWREF(obj);
  obj->type = type;

  return obj;
}

static void
_njord_free_object(NjRefsObject* obj) {
  if (obj->type == VAR_PAIR) {
    Nj_XDECREF(obj->head);
    Nj_XDECREF(obj->tail);
  }
  njmem_free(obj, sizeof(NjObject));
}

static void njref_pop(NjObject* vm);

static NjObject*
njref_new(void) {
  NjRefsVM* vm = (NjRefsVM*)njmem_malloc(sizeof(NjRefsVM));
  Nj_CHECK(vm != NULL, "create NjVM failed");

  vm->stackcnt = 0;

  return (NjObject*)vm;
}

static void
njref_free(NjObject* vm) {
  while (((NjRefsVM*)vm)->stackcnt > 0)
    njref_pop(vm);
  njmem_free(vm, sizeof(NjRefsVM));
}

static NjObject*
njref_pushint(NjObject* vm, int value) {
  NjRefsObject* obj = _njord_new_object(VAR_INT);
  obj->value = value;
  _njord_push((NjRefsVM*)vm, obj);

  return (NjObject*)obj;
}

static NjObject*
njref_pushpair(NjObject* vm) {
  NjRefsObject* obj = _njord_new_object(VAR_PAIR);
  NjRefsObject* tail = _njord_pop((NjRefsVM*)vm);
  NjRefsObject* head = _njord_pop((NjRefsVM*)vm);

  Nj_INCREF(head);
  obj->head = head;
  Nj_DECREF(head);

  Nj_INCREF(tail);
  obj->tail = tail;
  Nj_DECREF(tail);

  _njord_push((NjRefsVM*)vm, obj);

  return (NjObject*)obj;
}

static void
njref_setpair(NjObject* pair, NjObject* head, NjObject* tail) {
  if (head != NULL) {
    Nj_DECREF(((NjRefsObject*)pair)->head);
    Nj_INCREF(head);
    ((NjRefsObject*)pair)->head = (NjRefsObject*)head;
  }

  if (tail != NULL) {
    Nj_DECREF(((NjRefsObject*)pair)->tail);
    Nj_INCREF(tail);
    ((NjRefsObject*)pair)->tail = (NjRefsObject*)tail;
  }
}

static void
njref_pop(NjObject* vm) {
  NjRefsObject* obj = _njord_pop((NjRefsVM*)vm);
  Nj_DECREF(obj);
}

static void
njref_collect(NjObject* vm) {
  Nj_UNUSED(vm);
}

static NjGCMethods refs_methods = {
  njref_new, /* tp_newvm */
  njref_free, /* tp_freevm */
  njref_pushint, /* tp_pushint */
  njref_pushpair, /* tp_pushpair */
  njref_setpair, /* tp_setpair */
  njref_pop, /* tp_pop */
  njref_collect, /* tp_collect */
};

NjGCObject NjGC_Refs = {
  GC_REFS, /* type */
  (NjGCMethods*)&refs_methods, /* methods */
};
