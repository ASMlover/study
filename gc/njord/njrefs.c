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
#include "njrefs.h"

#define MAX_STACK (1024)

#define Nj_INCREF(ob) ((NjObject*)(ob))->refcnt++
#define Nj_DECREF(ob) do {\
  if (--((NjObject*)(ob))->refcnt == 0) {\
    fprintf(stdout, "NjObject(%p: %d) collected\n", ((NjObject*)(ob)), ((NjObject*)(ob))->type);\
    free(ob);\
  }\
} while (0)

struct _vm {
  NjObject* stack[MAX_STACK];
  int stackcnt;
};

static void
_njord_push(NjVM* vm, NjObject* obj) {
  Nj_CHECK(vm->stackcnt < MAX_STACK, "VM stack overflow");
  vm->stack[vm->stackcnt++] = obj;
}

static NjObject*
_njord_pop(NjVM* vm) {
  Nj_CHECK(vm->stackcnt > 0, "VM stack underflow");
  return vm->stack[--vm->stackcnt];
}

static NjObject*
_njord_new_object(NjType type) {
  NjObject* obj = (NjObject*)malloc(sizeof(NjObject));
  obj->refcnt = 1;
  obj->type = type;

  return obj;
}

static void
_njord_free_object(NjObject* obj) {
  if (obj->type == OBJECT_PAIR) {
    _njord_free_object(obj->head);
    _njord_free_object(obj->tail);
  }
  Nj_DECREF(obj);
}

NjVM*
njord_new(void) {
  NjVM* vm = (NjVM*)malloc(sizeof(NjVM));
  Nj_CHECK(vm != NULL, "create NjVM failed");

  vm->stackcnt = 0;

  return vm;
}

void
njord_free(NjVM* vm) {
  while (vm->stackcnt > 0)
    njord_pop(vm);
  free(vm);
}

NjObject*
njord_pushint(NjVM* vm, int value) {
  NjObject* obj = _njord_new_object(OBJECT_INT);
  obj->value = value;
  _njord_push(vm, obj);

  return obj;
}

NjObject*
njord_pushpair(NjVM* vm) {
  NjObject* obj = _njord_new_object(OBJECT_PAIR);
  NjObject* head = _njord_pop(vm);
  NjObject* tail = _njord_pop(vm);

  Nj_INCREF(head);
  obj->head = head;
  Nj_DECREF(head);

  Nj_INCREF(tail);
  obj->tail = tail;
  Nj_DECREF(tail);

  _njord_push(vm, obj);

  return obj;
}

void
njord_pop(NjVM* vm) {
  NjObject* obj = _njord_pop(vm);
  _njord_free_object(obj);
}

void
njord_collect(NjVM* vm) {
  Nj_UNUSED(vm);
}
